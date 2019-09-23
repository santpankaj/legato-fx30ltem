.PHONY: all pdf clean rtos
all: docs
docs:
	TMPL_RES_DIR="legato" ./build.sh --doc-tweaks
rtos:
	INPUT_DIR="$(LEGATO_ROOT)/build/doc/RTOS/html" TMPL_RES_DIR="rtos" ./build.sh
pdf:
	TMPL_RES_DIR="legato" ./build.sh --doc-tweaks --pdf
clean:
	rm -rf ../ConvertedDocs/
