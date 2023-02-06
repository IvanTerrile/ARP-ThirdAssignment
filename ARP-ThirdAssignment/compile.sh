# Compile process A
gcc src/processA.c -lpthread -lncurses -lbmp -lm -o bin/processA &

# Compile process B
gcc src/processB.c -lpthread -lncurses -lbmp -lm -o bin/processB &

# Compile master process
gcc src/master.c -o bin/master