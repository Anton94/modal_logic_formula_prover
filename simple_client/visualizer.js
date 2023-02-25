window.do_visualization_v2 = function do_visualization_v2(graph, width, height) {
    d3.select("svg").empty();
    d3.select("#visualization-graph").select("svg").remove();
    
    var svg = d3.select('#visualization-graph').append('svg').attr("viewBox", [0, 0, width, height]),
        // width = +svg.attr('width'),
        // height = +svg.attr('height'),
        color = d3.scaleOrdinal(d3.schemeCategory20),
        valueline = d3.line()
          .x(function(d) { return d[0]; })
          .y(function(d) { return d[1]; })
          .curve(d3.curveCatmullRomClosed),
        paths,
        groups,
        groupIds,
        scaleFactor = 1.8,
        polygon,
        centroid,
        node,
        link,
        curveTypes = ['curveBasisClosed', 'curveCardinalClosed', 'curveCatmullRomClosed', 'curveLinearClosed'],
        simulation = d3.forceSimulation()
          .force('link', d3.forceLink().id(function(d) { return d.id; }).distance(function (d) {
                return 150;
            }).strength(0.9))
          .force('charge', d3.forceManyBody())
          .force('center', d3.forceCenter(width / 2, height / 2));


    function start() {
      // create groups, links and nodes
      groups = svg.append('g').attr('class', 'groups');

      link = svg.append('g')
          .attr('class', 'links')
        .selectAll('line')
        .data(graph.links)
        .enter().append('line')
          .attr('stroke-width', function(d) { return 3; });

      node = svg.append('g')
          .attr('class', 'nodes')
        .selectAll('circle')
        .data(graph.nodes)
        .enter().append('circle')
          .attr('r', 4)
          .attr('fill', function(d) { return color(1); })
          .call(d3.drag()
              .on('start', dragstarted)
              .on('drag', dragged)
              .on('end', dragended));

      var label = svg.append('g')
        .attr("class", "labels")
        .selectAll("text")
        .data(graph.nodes)
        .enter().append("text")
        .attr("class", "label")
        .text(function (d) {
            return d.name;
        })
        .call(d3.drag()
            .on("start", dragstarted)
            .on("drag", dragged)
            .on("end", dragended));

      // count members of each group. Groups with less
      // than 3 member will not be considered (creating
      // a convex hull need 3 points at least)
      groupIds = [...new Set(graph.nodes.map(function(n) { return n.groups; }).flat())];

      paths = groups.selectAll('.path_placeholder')
        .data(groupIds, function(d) { return +d; })
        .enter()
        .append('g')
        .attr('class', 'path_placeholder')
        .append('path')
        .attr('stroke', function(d) { return color(d); })
        .attr('fill', function(d) { return color(d); })
        .attr('opacity', 0);

      paths
        .transition()
        .duration(2000)
        .attr('opacity', 1);

      // add interaction to the groups
      groups.selectAll('.path_placeholder')
        .attr("cursor", "grab")
        .call(d3.drag()
          .on('start', group_dragstarted)
          .on('drag', group_dragged)
          .on('end', group_dragended)
          );

      simulation
          .nodes(graph.nodes)
          .on('tick', ticked)
          .force('link')
          .links(graph.links);

      function ticked() {
        link
            .attr('x1', function(d) { return d.source.x; })
            .attr('y1', function(d) { return d.source.y; })
            .attr('x2', function(d) { return d.target.x; })
            .attr('y2', function(d) { return d.target.y; });
        node
            .attr('cx', function(d) { return d.x; })
            .attr('cy', function(d) { return d.y; });
       
        label
            .attr("x", function (d) {
                return d.x - 12;
            })
            .attr("y", function (d) {
                return d.y - 5;
            })
            .style("font-size", "20px").style("fill", "#346f93");
        updateGroups();
      }
    };

    start();

    // select nodes of the group, retrieve its positions
    // and return the convex hull of the specified points
    // (3 points as minimum, otherwise returns null)
    var polygonGenerator = function(groupId) {
      var node_coords = node
        .filter(function(d) { return d.groups.includes(groupId); })
        .data()
        .map(function(d) { return [d.x, d.y]; });
        
      if (node_coords.length == 1) {
        node_coords.push([node_coords[0][0] + 10, node_coords[0][1] + 10]);
        node_coords.push([node_coords[0][0] + 10, node_coords[0][1] - 10]);
        node_coords.push([node_coords[0][0] - 10, node_coords[0][1] + 10]);
        node_coords.push([node_coords[0][0] - 10, node_coords[0][1] - 10]);
      } else if (node_coords.length == 2) {
        node_coords.push([(node_coords[0][0] + node_coords[1][0])/2 + 25, (node_coords[0][1] + node_coords[1][1])/2 + 25])
      }
      return d3.polygonHull(node_coords);
    };

    function updateGroups() {
      groupIds.forEach(function(groupId) {
        var path = paths.filter(function(d) { return d == groupId;})
          .attr('transform', 'scale(1) translate(0,0)')
          .attr('d', function(d) {
            polygon = polygonGenerator(d);          
            centroid = d3.polygonCentroid(polygon);

            // to scale the shape properly around its points:
            // move the 'g' element to the centroid point, translate
            // all the path around the center of the 'g' and then
            // we can scale the 'g' element properly
            return valueline(
              polygon.map(function(point) {
                return [  point[0] - centroid[0], point[1] - centroid[1] ];
              })
            );
          });

        d3.select(path.node().parentNode).attr('transform', 'translate('  + centroid[0] + ',' + (centroid[1]) + ') scale(' + scaleFactor + ')');
      });
    }

    // drag nodes
    function dragstarted(d) {
      if (!d3.event.active) simulation.alphaTarget(0.3).restart();
      d.fx = d.x;
      d.fy = d.y;
    }

    function dragged(d) {
      d.fx = d3.event.x;
      d.fy = d3.event.y;
    }

    function dragended(d) {
      if (!d3.event.active) simulation.alphaTarget(0);
      d.fx = null;
      d.fy = null;
    }

    // drag groups
    function group_dragstarted(groupId) {
      if (!d3.event.active) simulation.alphaTarget(0.3).restart();
      d3.select(this).select('path').style('stroke-width', 3);
    }

    function group_dragged(groupId) {
      node
        .filter(function(d) { return d.groups.includes(groupId); })
        .each(function(d) {
          d.x += d3.event.dx;
          d.y += d3.event.dy;
        })
    }

    function group_dragended(groupId) {
      if (!d3.event.active) simulation.alphaTarget(0.3).restart();
      d3.select(this).select('path').style('stroke-width', 1);
    }

    function zoomed() {
        d3.selectAll('g').attr("transform", d3.event.transform);
    };
}

window.transform_contacts_to_graph = function transform_contacts_to_graph(ids, contacts) {
    nodes_temp = [...Array(contacts.length).keys()]

    nodes = [];
    for (i = 0; i < nodes_temp.length; ++i) {
        var x = nodes_temp[i];
        nodes.push({
            "id": x,
            "name": "A" + x,
            "groups": ids.map(a => a[i]).map((a, i) => a == 1 ? i : -1).filter(a => a >= 0)
        });
    }

    links = [];

    for (i = 0; i < contacts.length; ++i) {
        for (j = 0; j < contacts[i].length && j <= i; ++j) {
            if (contacts[i][j] == "1") {
                links.push({
                    "source": i,
                    "target": j
                });
            }
        }
    }

    return {
        "nodes": nodes,
        "links": links
    }
}