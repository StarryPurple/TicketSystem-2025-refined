# TicketSystem-2025
ACM Class 2025 Project

听说模板实现文件的后缀一般使用 ipp (inline) 而非 tcc (template)，

有缘再改~

Source: https://github.com/ACMClassCourse-2024/Ticket-System-2025

---
Yes I used some CMake tricks to make all files in a library for every file to refer to.
Maybe it's a little inappropriate?
---

### Overall view

The entrance file `main.cpp` is on the outside.

The `include` directory files with extension name `.h` store the declarations (and some definitions). 

The `src` directory files store with extension name `.cpp` definitions of non-template classes/functions.

The `template` directory files with extension name `.tcc` store definitions of template classes/functions.

You only need to look into `include` directory for useful functions.

---

### What's included in `include`

`algorithm`: simple algorithms like quicksort, hash, and string <-> integer conversions.

`common`: exceptions, and a string cache class called Messenger.

`container`: useful stl-style containers.

`database`: B+ Tree Index System on disk.

`disk`: Sector-aligned file manager.

`pool`: A buffer pool for disk cache, and an index pool for disk-block index allocation.

`ticketsystem` : Files related with the ticket system backend.