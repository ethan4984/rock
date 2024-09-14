### Memory Portals

```c
struct portal_req {
    int type;
    int prot;
    int length;

    struct __attribute__((packed)) {
        const char *identifier;
        int type;
        int create;
    } share;

    struct {
        const char *name;
        const char *message;
    } sp_req_chain;

    struct __attribute__((packed)) {
        uintptr_t addr;
        size_t length;
        uint64_t paddr[];
    } morphology;
} __attribute__((packed));

### Class

Often certain portal classifications are only accessible by special categories of server. A memory portal may belong to many such classifications and combinations of:

- **Share**: Establishes a region of shared memory between servers used for special types of protected IPC.
- **Direct**: Is used to create a direct map for a specified region in physical memory, mainly used for mapping MMIO.
- **Anon**: Creates an anonymous mapping of a specified size.
- **Cow**: A set of frames and references to be fulfilled in accordance with the principles of CoW (Copy on Write).
- **Special**: A special mapping for whose contents must be inferred from a specified chain of servers once written to or read from.

  To resolve a page fault pertaining to a special page, the running thread must be blocked, and upon the response, the server is unblocked.

These are all special operations that require special permissions to invoke.

### Trivial Share-Point

Passing objects over shared memory is extremely powerful if it's wrapped around the right protocols and interfaces to make it a little less chaotic.

Certain applications require less safety and assurance than others. For example, a unidirectional queue going from user-space to kernel-space. Another trivial but common application is shared meta-data between multiple instances of the same server, requiring only locking. These basic applications are all that is required for the multi-server scheduling interface to function.

- Under these cases, it is both the client’s and server’s responsibility to understand the nature of the data passed over shared memory. FAYT will provide macros and wrappers for data access to ensure locking.

- There will exist a table of shared memory portals, maintained in kernel space, each with an identifier, a list of the captured threads, and a pointer to the physical memory of the shared object. Each thread, in effect, manages its own virtual address space, so it will decide where it wants the share-point to be established.

- To create a share-point, you will call the portal system call, passing both the ANON (or DIRECT) and SHARE flags. You will provide a name identifying the share-point and pass a proper morphology. Then the caller will populate it with the share objects. Among any share-point, the first bytes will always be a meta-structure understood by all parties to be a governing object used for synchronization, defined as:

    ```c
    struct [[gnu::packed]] {
        char lock;
        int prot;
        size_t length;
        struct {
            size_t length;
            uint64_t paddr[];
        } morphology;
    };
    ```

    The share-point starts at the next 16-byte aligned address after this structure. All access to the share-point will be understood to only be accessed by a set of wrappers that ensure the locking, protection, and boundary conditions are respected.

- More advanced applications, such as bidirectional queues between servers, will require a level of validation akin to that of Unix domain sockets. A rigorous connection between client and server must be established and maintained. For something like this, which would involve frequent blocking, a capability that can only be provided by an advanced set of schedulers. So, I will dive deeper into this design later.

### Advanced Share-Point

TBD
