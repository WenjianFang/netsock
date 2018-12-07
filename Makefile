all:
	make -C lib/
	make -C apps/app-unix-client/
	make -C apps/app-unix-server/

install:
	make install -C lib/

clean:
	make clean -C lib/
	make clean -C apps/app-unix-client/
	make clean -C apps/app-unix-server/