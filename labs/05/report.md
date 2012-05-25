James Pearson (jmpearso)
CPE 453
Lab 5

1. The probability that all four programs are blocked at the same time is
`(1/2)**4 = .0625`.
2. a) 8020
   b) 4100
   c) 24300
3. The offset is `32 - 9 - 11 = 12` bits.  `2**12 = 4096`, so our pages are 4
KB.  `2**9 * 2**11 = 2**20` pages.
4. a) NRU replaces Page 0.
   b) FIFO replaces Page 2.
   c) LRU replaces Page 1.
   d) Second-chance replaces Page 0.
5. Using FIFO page replacement, there will be two page faults.
6. Using LRU page replacement, there will be four page faults.
7. 15000 page faults at 2000 microseconds each is 30 seconds; thus, the program
runs in 60-30=30 seconds minus page faults overhead.  If we double memory and
halve the number of page faults, the overhead should only be 15 seconds,
leading to a total program runtime of 45 seconds.

