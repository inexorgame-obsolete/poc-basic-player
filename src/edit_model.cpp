#include <memory>

// #define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

struct Face {
    float vertices[4];
    float normal[3];
};

struct PrimitiveCube {
    Face faces[6];
};

/*
Primitive + Primitive = Model (all triangles?)

Model -> VBO + Mesh
traverse nodes (depth first)
 */

/// Operations one can do with volumes/areas to get subsets/supersets.
/// Always taking the two volumes/areas A (left) and B (right) as operands.
enum OPERATION {
    NO_OP,          //< Indicate that this is not an operation, i.e. a leaf is reached.
    UNION,          //< Those parts, which are in either A, B or in both.
    INTERSECTION,   //< Subset: The part of two volumes, which are in both.
    SUBSTRACTION    //< Subset: The part of volume A, which is not in volume B
};

struct Node {
    OPERATION operation;
    /// The parent node is the owner of its child nodes.
    /// It has childs if operation is not NO_OP, in which case primitive is not nullptr.
    Node const *left, *right;

    /// This points to the primitive if this node is a leaf (i.e. operation is NO_OP).
    /// TODO: The node should not own the primitive, since many Trees could exist simultaneously and
    /// the primitive should not be needed to be copied then.-> make it a weak ptr and let it be owned somewhere else.
    PrimitiveCube *primitive;

    /*
    Node(const OPERATION op, const std::shared_ptr<Node> &l, const std::shared_ptr<Node> &r) :
            operation(op), left(nullptr), right(nullptr), primitive(nullptr)
    {
        // copy the old stuff!
        if(l) left = std::make_shared<Node>(*l);
        if(r) right = std::make_shared<Node>(*r);
    }
    explicit Node(const Node &n): operation(n.operation), left(nullptr), right(nullptr), primitive(nullptr)
    {
        // copy the old stuff!
        if(n.left) left = std::make_shared<Node>(*n.left);
        if(n.right) right = std::make_shared<Node>(*n.right);
        if(n.primitive) primitive = std::make_shared<PrimitiveCube>(*n.primitive);
    }
    Node(Node &&) = default;

     */
};

Node *create_new_node(const OPERATION op, const Node *l, const Node *r)
{
    // copy the old stuff!
    Node *n = new Node();
    n->primitive = nullptr;
    n->left = l ? new Node(*l) : nullptr;
    n->right = r ? new Node(*r) : nullptr;
    return n;
}

Node *create_new_simplified_node(Node const * const node)
{
    /// The Goldfeather Algorithm
    /// Taken from OpenCSG: A Library for Image-Based CSG Rendering, 2.2
    /// https://www.usenix.org/legacy/event/usenix05/tech/freenix/full_papers/kirsch/kirsch.pdf
    // assert(node);
    if (!node->left || !node->right) {
        return new Node(*node);
    }

    // - = Substraction
    // a = Intersection
    // u = Union

    // Rule 1: x - (y u z) ---> (x - y) - z
    if (node->operation == SUBSTRACTION &&
        node->right->operation == UNION)
    {
        auto new_left_node = create_new_node(SUBSTRACTION, node->left, node->right->left);
        return create_new_node(SUBSTRACTION, new_left_node, node->right->right);
    }

    // Rule 2: x a (y u z) ---> (x a y) u (x a z)
    if (node->operation == INTERSECTION &&
        node->right->operation == UNION)
    {
        auto new_left_node = create_new_node(INTERSECTION, node->left, node->right->left);
        auto new_right_node = create_new_node(INTERSECTION, node->left, node->right->right);
        return create_new_node(UNION, new_left_node, new_right_node);
    }
    // Rule 3: x - (y a z) ---> (x - y) u (x - z)
    if (node->operation == SUBSTRACTION &&
        node->right->operation == INTERSECTION)
    {
        auto new_left_node = create_new_node(SUBSTRACTION, node->left, node->right->left);
        auto new_right_node = create_new_node(SUBSTRACTION, node->left, node->right->right);
        return create_new_node(UNION, new_left_node, new_right_node);
    }

    // Rule 4: x a (y a z) ---> (x a y) a z
    if (node->operation == INTERSECTION &&
        node->right->operation == INTERSECTION)
    {
        auto new_left_node = create_new_node(INTERSECTION, node->left, node->right->left);
        return create_new_node(INTERSECTION, new_left_node, node->right->right);
    }

    // Rule 5: x - (y - z) ---> (x - y) u (x a z)
    if (node->operation == SUBSTRACTION &&
        node->right->operation == SUBSTRACTION)
    {
        auto new_left_node = create_new_node(SUBSTRACTION, node->left, node->right->left);
        auto new_right_node = create_new_node(INTERSECTION, node->left, node->right->right);
        return create_new_node(UNION, new_left_node, new_right_node);
    }

    // Rule 6: x a (y - z) ---> (x a y) - z
    if (node->operation == INTERSECTION &&
        node->right->operation == SUBSTRACTION)
    {
        auto new_left_node = create_new_node(SUBSTRACTION, node->left, node->right->left);
        return create_new_node(SUBSTRACTION, new_left_node, node->right->right);
    }

    // Rule 7: (x-y) a z ---> (x a z) - y
    if (node->operation == INTERSECTION &&
        node->left->operation == SUBSTRACTION)
    {
        auto new_left_node = create_new_node(INTERSECTION, node->left->left, node->right);
        return create_new_node(SUBSTRACTION, new_left_node, node->right);
    }

    // Rule 8: (x u y) - z ---> (x - z) u (y - z)
    if (node->operation == SUBSTRACTION &&
        node->left->operation == UNION)
    {
        auto new_left_node = create_new_node(SUBSTRACTION, node->left->left, node->right);
        auto new_right_node = create_new_node(SUBSTRACTION, node->left->right, node->right);
        return create_new_node(UNION, new_left_node, new_right_node);
    }

    // Rule 9: (x u y) a z ---> (x a z) u (y a z)
    if (node->operation == INTERSECTION &&
        node->left->operation == UNION)
    {

        auto new_left_node = create_new_node(INTERSECTION, node->left->left, node->right);
        auto new_right_node = create_new_node(INTERSECTION, node->left->right, node->right);
        return create_new_node(UNION, new_left_node, new_right_node);
    }
}

/// Always replace the childs if you got any (recursively).
Node *create_normalized_node(Node const * const current_node)
{
    if (current_node->operation == NO_OP)
        // no children to copy
        return new Node(*current_node);;

    // be recursive: if they got childs, replace them first.
    Node *left = create_normalized_node(current_node->left);
    Node *right = create_normalized_node(current_node->right);
    Node *cloned_node = create_new_simplified_node(current_node);
    cloned_node->left = left;
    cloned_node->right = right;
    return cloned_node;
}

Node *create_normalized_tree(Node const * const tree_root)
{
    // assert(tree_root != nullptr);
    Node *new_tree_root = create_normalized_node(tree_root);
    return new_tree_root;
    // walk the tree in postorder
    //for(const Node *ptr = tree_root;  ptr->right != nullptr; ptr = ptr->right) {
        //    create_normalized_node(ptr);
        //}

    // auto ptr = tree_root;
    // for(ptr; ptr->right != nullptr; ptr = ptr->right) {}
}

TEST_CASE("Testing normalizing a tree") {
    // setup
    auto leaf_node_1 = create_new_node(NO_OP, nullptr, nullptr);
    auto leaf_node_2 = create_new_node(NO_OP, nullptr, nullptr);
    auto op_node_1 = create_new_node(UNION, leaf_node_1, leaf_node_2);
    auto leaf_node_3 = create_new_node(NO_OP, nullptr, nullptr);
    auto leaf_node_4 = create_new_node(NO_OP, nullptr, nullptr);
    auto op_node_2 = create_new_node(UNION, leaf_node_3, leaf_node_4);
    auto leaf_node_5 = create_new_node(NO_OP, nullptr, nullptr);
    auto leaf_node_6 = create_new_node(NO_OP, nullptr, nullptr);
    auto op_node_3 = create_new_node(INTERSECTION, leaf_node_5, leaf_node_6);
    // this operation is an operation between only non-leaves! Thats what we want to erase.
    auto op_node_fat = create_new_node(UNION, op_node_3, op_node_2);
    auto op_node_fat_2 = create_new_node(UNION, op_node_fat, op_node_1);

    // lets see whether the tree is normalized afterwards
    auto result = create_normalized_tree(op_node_fat_2);

    for(const Node *ptr = result;  ptr->left != nullptr; ptr = ptr->left) {
        DOCTEST_CHECK(ptr->right->operation == NO_OP);
        DOCTEST_CHECK(ptr)
    };
}