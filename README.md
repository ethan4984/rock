# crepOS
an OS made for fun and learning

**Features:**

  - k_print and t_print; t_print outputs to serial port COM1 from debugging, k_print outputs to the screen

  - port manipulation eg..(in, out, serial)

  - IRQ1 works great so we have keyboard and a keyboard "driver"

**ToDo:**
  - full idt
  - isr
  - a shell
  - malloc/free
  - process scheduling
  - colors
  - graphics
  - windows
  - a lot more

**Build:**

  If you already have a cross compiler just
  run mkdir Bin to make the bin directory (just cuz that is useally dont in cross.sh if your not running it)

  Run the Tools/cross.sh script to setup the cross compiler (recomended)
  Then run the makefile to run it in qemu
  (Btw you might be missing some dependences for this idk, some but not all dependences include, mtools, xorriso, etc)
  
  Remember, every terminal that you want to run this in you have to run: 
  
      export PREFIX="$HOME/opt/cross"
      
      export TARGET=i686-elf
      
      export PATH="$PREFIX/bin:$PATH
      
      
      
      
