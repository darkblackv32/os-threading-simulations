chef:
	gcc -pthread -o chef chef.c && valgrind --tool=helgrind ./chef
cinema:
	gcc -pthread -o cine cine.c && valgrind --tool=helgrind ./cinema
	
# remove generated .txt files before recompiling
