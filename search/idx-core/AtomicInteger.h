#ifndef ATOMIC_H
#define ATOMIC_H

typedef uint32_t T;
class AtomicInteger
{
    public:
        AtomicInteger()
        {
        }

        AtomicInteger(T initial)
        {
            value=initial;
        }

        operator T() const
        {
            return get();
        }

        T inline get() const
        {
            return value;
        }

        void inline set(T i)
        {
            value=i;
        }

        void inline add(T i)
        {
            asm volatile("lock addl %1,%0"
                 : "+m" (value)
                 : "ir" (i));
        }

        void inline sub(T i)
        {
            asm volatile("lock subl %1,%0"
                 : "+m" (value)
                 : "ir" (i));
        }

        T inline sub_and_test(T i)
        {
            unsigned char c;

            asm volatile("lock subl %2,%0; sete %1"
                     : "+m" (value), "=qm" (c)
                     : "ir" (i) : "memory");
            return c;
        }

        void inline inc()
        {
            asm volatile("lock incl %0"
                : "+m" (value));
        }

        void inline dec()
        {
            asm volatile("lock decl %0"
                : "+m" (value));
        }

        T inline inc_and_test()
        {
            unsigned char c;

            asm volatile("lock incl %0; sete %1"
                     : "+m" (value), "=qm" (c)
                     : : "memory");
            return c != 0;
        }

        T inline dec_and_test()
        {
            unsigned char c;

            asm volatile("lock decl %0; sete %1"
                     : "+m" (value), "=qm" (c)
                     : : "memory");
            return c != 0;
        }

        T inline add_return(T i)
        {
            volatile T __i = i;
            asm volatile("lock xadd %0, %1"
                     : "+r" (i), "+m" (value)
                     : : "memory");
            return i + __i;
        }

        T inline inc_return()
        {
            return add_return(1);
        }

        T inline cmpxchg(T oldi, T newi)
        {
            volatile T prev;
            asm volatile("lock cmpxchgl %k1,%2"
                         : "=a"(prev)
                         : "r"(newi), "m"(value), "0"(oldi)
                         : "memory");
            return prev;
        }

        T inline xchg(T x)
        {
            asm volatile("xchgl %k0,%1"
                     : "=r" (x)
                     : "m" (value), "0" (x)
                     : "memory");
            return x;
        }

private:
        volatile T value;

        // AtomicInteger& operator = (const AtomicInteger& rhs);
        AtomicInteger(const AtomicInteger& rhs);
};
#endif
