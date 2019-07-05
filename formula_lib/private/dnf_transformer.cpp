#include "dnf_transformer.h"

dnf_transformer::dnf_node::dnf_node(const std::string& x, dnf_node* left, dnf_node* right)
    : x_(x)
    , left_(left)
    , right_(right)
{
}

dnf_transformer::dnf_node::~dnf_node()
{
    delete left_;
    delete right_;
}

auto dnf_transformer::transform_to_dnf_term(json& f) const -> dnf_term
{
    return dnf_term {};
}

// TODO refactor
//auto dnf_transformer::json_to_tree(json& f) -> dnf_node
//{
//    if(!f.contains("name"))
//    {
//        throw "fail";
//    }

//    auto& name_field = f["name"];
//    if(!name_field.is_string())
//    {
//        throw "Fail";
//    }

//    std::string op = name_field.get<std::string>();
//    if(op == "Tand" || op == "Tor")
//    {
//        auto& value_field = f["value"];
//        if(!value_field.is_array() || value_field.size() != 2)
//        {
//            throw "fail";
//        }
//        dnf_node& t_left = json_to_tree(value_field[0]);
//        dnf_node& t_right = json_to_tree(value_field[1]);
//        dnf_node* node = new dnf_node(op, &t_left, &t_right);
//        return *node;
//    }
//    else if(op == "Tstar")
//    {
//        auto& value_field = f["value"];
//        if(!value_field.is_object())
//        {
//            throw "fail";
//        }
//        dnf_node& t_left = json_to_tree(value_field);
//        dnf_node* node = new dnf_node(op, &t_left);
//        return *node;
//    }
//    else if(op == "string")
//    {
//        auto& value_field = f["value"];
//        if(!value_field.is_string())
//        {
//            throw "fail";
//        }
//        dnf_node* node = new dnf_node(value_field.get<std::string>());
//        return *node;
//    }
//    else
//    {
//        throw "fail";
//    }
//}

dnf_transformer::dnf_node& dnf_transformer::negative_normal_form(dnf_node& root)
{
    if(root.x_ == "Tand" || root.x_ == "Tor" || root.x_ == "Tstar")
    {
        negative_normal_form(*root.left_);
        if(root.x_ != "Tstar")
        {
            negative_normal_form(*root.right_);
        }
    }
    if(root.x_ == "Tstar")
    {
        if(root.left_->x_ == "Tstar")
        {
            // double star. remove the nodes only.
            dnf_node* temp = root.left_;
            root.x_ = root.left_->left_->x_;

            root.left_ = root.left_->left_->left_;
            if(root.x_ != "Tstar")
            {
                root.right_ = root.left_->left_->right_;
            }

            temp->left_->left_ = nullptr;
            temp->left_->right_ = nullptr;
            delete temp;
            negative_normal_form(root);
        }
        else if(root.left_->x_ == "Tor" || root.left_->x_ == "Tand")
        {
            if(root.left_ == nullptr)
            {
                throw "fail";
            }

            root.right_ = new dnf_node("Tstar", root.left_->right_);
            root.left_->right_ = nullptr;

            if(root.left_->x_ == "Tand")
            {
                root.x_ = "Tor";
            }
            else if(root.left_->x_ == "Tor")
            {
                root.x_ = "Tand";
            }
            else
            {
                throw "fail";
            }

            root.left_->x_ = "Tstar";
        }
    }
    return root;
}

//auto dnf_transformer::minimize_tree(dnf_node& root)
//{
//    return dnf_node(":A");
//}

//auto dnf_transformer::tree_to_dnf_term(dnf_node& root)
//{
//    return dnf_term();
//}
