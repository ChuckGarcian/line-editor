rm ldp
gcc -g -O0 -o ldp main.c TerminalLineEditor.c buffer.c EventDispatch/KeyPressEventDispatcher.c EventDispatch/queue.c  HQueue.c
