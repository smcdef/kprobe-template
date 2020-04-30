# kprobe-template

I usually use kprobe/kretprobe in a kernel module. But I usually do a lot of pointless things. A lot of CTRL+C and CTRL+V. I am fed up with these. So I want to use macro definitions to do these meaningless things. So this is why I did this. Enjoy yourself.

## How to install

```bash
git clone https://github.com/smcdef/kprobe-template.git
cd kprobe-template
make -j8
```

The `kretprobe.ko`is the module name in the kprobe-template. Using the following command to install it.

```bash
make install
```

If the module is installed successfully, you can get the following message via the `dmesg`.

```bash
kretprobe: planted kretprobe at inode_permission+0x0/0x180
```

If you want to uninstall the module, you can use the following command.

```bash
make remove
```

Also, the `dmesg` will output the following message.

```bash
kretprobe: kretprobe at inode_permission+0x0/0x180 unregistered
```

## How to use

The API is mainly divided into two categories, namely kprobe and kretprobe. First let's see how to use the kprobe APIs.

### kprobe

Now if you want to hook the `do_sys_open` function, what should you do. First of all, we can find the definition of the `do_sys_open` in the fs/open.c.

```c
long do_sys_open(int dfd, const char __user *filename,
                 int flags, umode_t mode);
```

In the kretprobe.h, there are seven APIs that we can use for kprobe.

```c
KPROBE_HANDLER_DEFINE0(function);
KPROBE_HANDLER_DEFINE1(function);
KPROBE_HANDLER_DEFINE2(function);
KPROBE_HANDLER_DEFINE3(function);
KPROBE_HANDLER_DEFINE4(function);
KPROBE_HANDLER_DEFINE5(function);
KPROBE_HANDLER_DEFINE6(function);

KPROBE_HANDLER_DEFINE_OFFSET(function, offset);
```

As you can see, the `do_sys_open` has four parameters. So we should use the KPROBE_HANDLER_DEFINE4. For the same reason, if the function has none parameter. You should use the KPROBE_HANDLER_DEFINE0. Then we should program as follows.

```c
KPROBE_HANDLER_DEFINE4(do_sys_open,
                       int, dfd, const char __user *, filename,
                       int, flags, umode_t, mode)
{
        /* Now you get all parameters. */
        pr_info("mode: %xn", mode);
        return 0;
}
```

If you want to kprobe a function at the special offet(e.g. 0x5). Just like this.

```c
KPROBE_HANDLER_DEFINE_OFFSET(do_sys_open, 0x5,
                             struct pt_regs *, regs)
{
        /*
         * The context registers are store in
         * the struct pt_regs.
         */
        return 0;
}
```

### kretprobe

In the kretprobe.h, there are seven APIs that we can use for kretprobe. Six of them are for entry handler, the other is for return handler.

```c
/* entry handler */
KRETPROBE_ENTRY_HANDLER_DEFINE0(func, type, arg);
KRETPROBE_ENTRY_HANDLER_DEFINE1(func, type, arg);
KRETPROBE_ENTRY_HANDLER_DEFINE2(func, type, arg);
KRETPROBE_ENTRY_HANDLER_DEFINE3(func, type, arg);
KRETPROBE_ENTRY_HANDLER_DEFINE4(func, type, arg);
KRETPROBE_ENTRY_HANDLER_DEFINE5(func, type, arg);
KRETPROBE_ENTRY_HANDLER_DEFINE6(func, type, arg);

/* return handler */
KRETPROBE_RET_HANDLER_DEFINE(func);
```

Suppose you want to trace `do_sys_open`, and you want to print the parameters passed by its caller only when `do_sys_open` returns an error. How to do that?

```c
struct parameters {
        const char __user *filename;
        int flags;
};

/* do_sys_open entry handler */
KRETPROBE_ENTRY_HANDLER_DEFINE4(do_sys_open, struct parameters *, pars,
                                int, dfd, const char __user *, filename,
                                int, flags, umode_t, mode)
{
        if (!current->mm)
                return -1;	/* Skip kernel threads */

        pars->filename = filename;
        pars->flags = flags;

        return 0;
}

/* do_sys_open return handler */
KRETPROBE_RET_HANDLER_DEFINE(do_sys_open,
                             struct parameters *, pars, int, retval)
{
        if (retval < 0)
                pr_info("flags: 0x%x, retval: %dn", pars->flags, retval);
        return 0;
}
```

The `do_sys_open` has four parameters, so you should use KRETPROBE_ENTRY_HANDLER_DEFINE4. The `struct parameters`is the your own private data structure. If you only want to store one parameter or other privata data(maybe timestamp or what else you want), you can just use a `long` type instead of a structure. The KRETPROBE_RET_HANDLER_DEFINE only has two parameters, one is the private data structure and the other is the `retval` which is the return value for the `do_sys_open` function.
