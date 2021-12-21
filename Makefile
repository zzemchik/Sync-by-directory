NAME = synhrone

all:
	g++ -std=c++17 .\main.cpp -o $(NAME)
clean:
	rm -rf $(NAME)
