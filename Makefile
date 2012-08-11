keytable: keytable.c parse.h

parse.h: /usr/include/linux/input.h
	@echo generating parse.h
	@echo -en "struct parse_key {\n\tchar *name;\n\tunsigned int value;\n} " >parse.h
	@echo -en "keynames[] = {\n" >>parse.h

	@more /usr/include/linux/input.h |perl -n \
	-e 'if (m/^\#define\s+(KEY_[^\s]+)\s+(0x[\d\w]+|[\d]+)/) ' \
	-e '{ printf "\t{\"%s\", %s},\n",$$1,$$2; }' \
	-e 'if (m/^\#define\s+(BTN_[^\s]+)\s+(0x[\d\w]+|[\d]+)/) ' \
	-e '{ printf "\t{\"%s\", %s},\n",$$1,$$2; }' \
	>> parse.h
	@echo -en "\t{ NULL, 0}\n};\n" >>parse.h
