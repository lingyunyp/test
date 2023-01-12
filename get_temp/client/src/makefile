
LIBPATH =`pwd`/../lib/
CFLAGS +=-I`pwd`/../inc/
LDFLAGS += -L${LIBPATH} 
NAME = client

all:clean create_o
	gcc `pwd`/*.o -o ${NAME} ${CFLAGS} ${LDFLAGS} -lsqlite3  

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
