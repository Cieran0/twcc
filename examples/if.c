void printf(int s);

int say_hey(int should_say_hey) {
    if(should_say_hey + 0) {
        printf("Hey!\n");
        return 1;
    }
    return 0;
}