struct node {
    bool terminal;
    bool nil;
    int type;

    struct node* left_child;
    struct node* right_child;

    struct token* token;
};