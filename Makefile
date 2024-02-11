all:
	gcc yaml.c process.c main.c -lyaml -o main

test:
	gcc test.c -g -o test

qa: yaml process

process:
	gcc -DQA process.c -g -o process && ./process

yaml:
	gcc -DQA yaml.c -lyaml -g -o yaml && ./yaml

.PHONY: test all yaml
