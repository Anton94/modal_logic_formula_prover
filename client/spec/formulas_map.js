mock_json_by_formula = {

	"(C(a, b) & ~C(c, 0)) & <=(1, 0)": JSON.parse(
		'{ \
		   "name": "conjunction", \
		   "value": [ \
		      { \
		         "name": "conjunction", \
		         "value": [ \
		            { \
		               "name": "contact", \
		               "value": [ \
		                  { \
		                     "name": "string", \
		                     "value": "a" \
		                  }, \
		                  { \
		                     "name": "string", \
		                     "value": "b" \
		                  } \
		               ] \
		            }, \
		            { \
		               "name": "negation", \
		               "value": { \
		                  "name": "contact", \
		                  "value": [ \
		                     { \
		                        "name": "string", \
		                        "value": "c" \
		                     }, \
		                     { \
		                        "name": "0" \
		                     } \
		                  ] \
		               } \
		            } \
		         ] \
		      }, \
		      { \
		         "name": "equal0", \
		         "value": { \
		            "name": "Tand", \
		            "value": [ \
		               { \
		                  "name": "1" \
		               }, \
		               { \
		                  "name": "Tstar", \
		                  "value": { \
		                      "name": "0" \
		                  } \
		               } \
		            ] \
		         } \
		      } \
		   ] \
		}'
	),

	"C(a, b)": JSON.parse(
		'{ \
	        "name": "contact", \
	        "value": [ \
	           { \
	              "name": "string", \
	              "value": "a" \
	           }, \
	           { \
	              "name": "string", \
	              "value": "b" \
	           } \
	        ] \
	    }'
	),

	"C(a, b) & ~C(a,b)": JSON.parse(
		'{ \
		   "name": "conjunction", \
		   "value": [ \
		      { \
		         "name": "contact", \
		         "value": [ \
		            { \
		               "name": "string", \
		               "value": "a" \
		            }, \
		            { \
		               "name": "string", \
		               "value": "b" \
		            } \
		         ] \
		      }, \
		      { \
		         "name": "negation", \
		         "value": { \
		            "name": "contact", \
		            "value": [ \
		               { \
		                  "name": "string", \
		                  "value": "a" \
		               }, \
		               { \
		                  "name": "string", \
		                  "value": "b" \
		               } \
		            ] \
		         } \
		      } \
		   ] \
		}'
	),

	"(C(a, b) | <=(a, b)) | ~C(a,b)": JSON.parse(
		'{ \
		  "name": "disjunction", \
		  "value": [ \
		     { \
		        "name": "disjunction", \
		        "value": [ \
		           { \
		              "name": "contact", \
		              "value": [ \
		                 { \
		                    "name": "string", \
		                    "value": "a" \
		                 }, \
		                 { \
		                    "name": "string", \
		                    "value": "b" \
		                 } \
		              ] \
		           }, \
		           { \
		              "name": "equal0", \
		              "value": { \
		                 "name": "Tand", \
		                 "value": [ \
		                    { \
		                       "name": "string", \
		                       "value": "a" \
		                    }, \
		                    { \
		                       "name": "Tstar", \
		                       "value": { \
		                          "name": "string", \
		                          "value": "b" \
		                       } \
		                    } \
		                 ] \
		              } \
		           } \
		        ] \
		     }, \
		     { \
		        "name": "negation", \
		        "value": { \
		           "name": "contact", \
		           "value": [ \
		              { \
		                 "name": "string", \
		                 "value": "a" \
		              }, \
		              { \
		                 "name": "string", \
		                 "value": "b" \
		              } \
		           ] \
		        } \
		     } \
		  ] \
		}'
	),

	"(C(a, b) | <=(a, b)) | (~C(a,b) & <=(a,b))": JSON.parse(
		'{ \
		   "name": "disjunction", \
		   "value": [ \
		      { \
		         "name": "disjunction", \
		         "value": [ \
		            { \
		               "name": "contact", \
		               "value": [ \
		                  { \
		                     "name": "string", \
		                     "value": "a" \
		                  }, \
		                  { \
		                     "name": "string", \
		                     "value": "b" \
		                  } \
		               ] \
		            }, \
		            { \
		               "name": "equal0", \
		               "value": { \
		                  "name": "Tand", \
		                  "value": [ \
		                     { \
		                        "name": "string", \
		                        "value": "a" \
		                     }, \
		                     { \
		                        "name": "Tstar", \
		                        "value": { \
		                           "name": "string", \
		                           "value": "b" \
		                        } \
		                     } \
		                  ] \
		               } \
		            } \
		         ] \
		      }, \
		      { \
		         "name": "conjunction", \
		         "value": [ \
		            { \
		               "name": "negation", \
		               "value": { \
		                  "name": "contact", \
		                  "value": [ \
		                     { \
		                        "name": "string", \
		                        "value": "a" \
		                     }, \
		                     { \
		                        "name": "string", \
		                        "value": "b" \
		                     } \
		                  ] \
		               } \
		            }, \
		            { \
		               "name": "equal0", \
		               "value": { \
		                  "name": "Tand", \
		                  "value": [ \
		                     { \
		                        "name": "string", \
		                        "value": "a" \
		                     }, \
		                     { \
		                        "name": "Tstar", \
		                        "value": { \
		                           "name": "string", \
		                           "value": "b" \
		                        } \
		                     } \
		                  ] \
		               } \
		            } \
		         ] \
		      } \
		   ] \
		}'
	),
}

module.exports = { mock_json_by_formula }
