build:
	gcc -o twcc main.c pre_token_arena.c symbol_table.c -Iinclude

run:
	gcc -o twcc main.c pre_token_arena.c symbol_table.c -Iinclude
	./twcc