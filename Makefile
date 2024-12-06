.PHONY: clean all compile

# 기본 컴파일러를 ARM64 크로스 컴파일러로 설정
#CC = gcc
CC = aarch64-linux-gnu-gcc

CFLAGS = -Ilib
SRC := $(wildcard src/*.c)
HDR := $(wildcard lib/*.h)
OBJ := $(patsubst src/%.c, Out/%.o, $(SRC))  # Out 폴더에 객체 파일 생성
OUT_DIR = Out

TARGET = main_server

# all 타겟에서 clean과 compile을 순서대로 실행
all: compile

# compile 타겟은 $(TARGET) 빌드를 수행
compile: $(OUT_DIR) $(TARGET)

# Out 디렉토리가 없으면 생성
$(OUT_DIR):
	mkdir -p $(OUT_DIR)

# 실행 파일 생성 규칙
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

# 개별 소스 파일을 컴파일하여 Out 폴더에 객체 파일 생성
Out/%.o: src/%.c $(HDR)
	$(CC) $(CFLAGS) -c $< -o $@

# .o 파일과 실행 파일을 삭제하는 clean 타겟
clean:
	rm -rf $(OUT_DIR) $(TARGET)
