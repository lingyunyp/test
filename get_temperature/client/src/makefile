
LIBPATH =`pwd`/../lib/
CFLAGS +=  -I`pwd`/../inc/
LDFLAGS += -L${LIBPATH} -lsqlite3 
NAME = client

all:clean create_o
	gcc -lm `pwd`/*.o -o ${NAME} ${CFLAGS} ${LDFLAGS} 

create_o:
	gcc -c `pwd`/*.c ${CFLAGS} ${LDFLAGS}


	
clean:
	rm -f ${NAME}

distclean:clean
	rm -f *.o 
	rm -f *.log
	rm -f *.db

run:
	export  LD_LIBRARY_PATH=${LIBPATH}
	#export PATH= $PATH:`pwd`/../inc/
