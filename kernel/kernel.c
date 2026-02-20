void kernel_main() {
    volatile char* video = (volatile char*) 0xB8000;

    video[0] = 'O';
    video[1] = 0x07;

    video[2] = 'S';
    video[3] = 0x07;

    while (1);
}