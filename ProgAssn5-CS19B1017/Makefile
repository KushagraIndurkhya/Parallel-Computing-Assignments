all: build run
build:
	@echo "=> Compiling"
	@mkdir -p bin
	@g++ TAS-CS19B1017.cpp --std=c++17 -pthread -o bin/TAS.out
	@g++ TTAS-CS19B1017.cpp --std=c++17 -pthread -o bin/TTAS.out
	@g++ CLH-CS19B1017.cpp --std=c++17 -pthread -o bin/CLH.out
	@g++ MCS-CS19B1017.cpp --std=c++17 -pthread -o bin/MCS.out
	@echo "=> Done \xE2\x9C\x94"

run:
	@echo "=> Running for Input..."
	@cat inp-params.txt
	@echo
	@./bin/TAS.out
	@./bin/TTAS.out
	@./bin/CLH.out
	@./bin/MCS.out

clean:
	@rm -f -r ./bin/ *_out.txt