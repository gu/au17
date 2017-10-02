struct node {
    bool terminal;
    bool nil;

    struct node* left_child;
    struct node* right_child;

    struct token* token;
};