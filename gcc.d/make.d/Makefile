CC:=gcc
#CC:= arm-linux-gnueabihf-gcc

OBJECT = input.o output.o hi.o  # 실행할 파일을 위해 사용되는 준비물 
TARGET = hi   # 실행할 목폭
$(TARGET):$(OBJECT)  #
	echo "Makefile"
	$(CC) -o $@ $(OBJECT)
#	cp $@ /srv/nfs_server

%.o:%.c
	$(CC) -c -o $@ $<
clean:
	rm -f *.o $(TARGET)
	echo "delete"

