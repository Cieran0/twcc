build:
	gcc -o twcc main.c pre_token_arena.c -Iinclude

run:
	gcc -o twcc main.c pre_token_arena.c -Iinclude
	./twcc