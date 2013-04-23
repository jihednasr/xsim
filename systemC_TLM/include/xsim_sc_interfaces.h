#ifndef _XSIM_INTERFACES_H_
#define _XSIM_INTERFACES_H_

template<typename T>
class write_if : virtual public sc_interface
{
    public:
        virtual void write(T data) = 0;
        virtual void reset() = 0;
};

template<typename T>
class read_if : virtual public sc_interface
{
    public:
        virtual void read(T & data) = 0;
        virtual int num_available() = 0;
};

#endif
