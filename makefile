# all: main.cc xq4ms logE
# 	g++ main.cc -o app -std=c++17 -g
# xq4ms: xq4ms.cc
# 	g++ xq4ms.cc -o xq4ms -std=c++17
# logE: log.cc
# 	g++ log.cc -o logE -std=c++17 -g
# clean:
#	$(RM) app xq4ms logE
# logclean:
# 	rm -rf ./log ./archive ./inference

all: main.cc
	g++ main.cc -o app -std=c++17 -g
clean:
	$(RM) app xq4ms logE
logclean:
	rm -rf ./log ./archive ./inference

