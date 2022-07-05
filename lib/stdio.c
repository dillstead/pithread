#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <uart.h>
#include "ctype.h"
#include "string.h"
#include "stdio.h"

struct printf_conversion 
{
    /* Flags. */
    enum 
    {
        MINUS = 1 << 0,         /* '-' */
        PLUS = 1 << 1,          /* '+' */
        SPACE = 1 << 2,         /* ' ' */
        POUND = 1 << 3,         /* '#' */
        ZERO = 1 << 4,          /* '0' */
        GROUP = 1 << 5          /* '\'' */
    } flags;
    
    /* Minimum field width. */
    int width;
    
    /* Numeric precision.
       -1 indicates no precision was specified. */
    int precision;
    
    /* Type of argument to format. */
    enum 
    {
        CHAR = 1,               /* hh */
        SHORT = 2,              /* h */
        INT = 3,                /* (none) */
        INTMAX = 4,             /* j */
        LONG = 5,               /* l */
        LONGLONG = 6,           /* ll */
        PTRDIFFT = 7,           /* t */
        SIZET = 8               /* z */
    } type;
};

struct integer_base 
{
    int base;                   /* Base. */
    const char *digits;         /* Collection of digits. */
    int x;                      /* `x' character to use, for base 16 only. */
    int group;                  /* Number of digits to group with ' flag. */
};

static const struct integer_base base_d = {10, "0123456789", 0, 3};
static const struct integer_base base_o = {8, "01234567", 0, 3};
static const struct integer_base base_x = {16, "0123456789abcdef", 'x', 4};
static const struct integer_base base_X = {16, "0123456789ABCDEF", 'X', 4};

static void __printf (const char *format,
                      void (*output)(char, void *), void *aux, ...);

/* Helper function for vprintf(). */
static void vprintf_helper(char c, void *char_cnt_) 
{
    int *char_cnt = char_cnt_;
    (*char_cnt)++;
    uart_putc(c);
}

/* Writes CH to OUTPUT with auxiliary data AUX, CNT times. */
static void output_dup (char ch, size_t cnt, void (*output)(char, void *), void *aux) 
{
    while (cnt-- > 0)
    {
        output(ch, aux);
    }
}

static const char *parse_conversion(const char *format, struct printf_conversion *c,
                                    va_list *args)
{
    /* Parse flag characters. */
    c->flags = 0;
    for (;;) 
    {
        switch (*format++) 
        {
        case '-':
        {
            c->flags |= MINUS;
            break;
        }
        case '+':
        {
            c->flags |= PLUS;
            break;
        }
        case ' ':
        {
            c->flags |= SPACE;
            break;
        }
        case '#':
        {
            c->flags |= POUND;
            break;
        }
        case '0':
        {
            c->flags |= ZERO;
            break;
        }
        case '\'':
        {
            c->flags |= GROUP;
            break;
        }
        default:
        {
            format--;
            goto not_a_flag;
        }
        }
    }
not_a_flag:
    if (c->flags & MINUS)
    {
        c->flags &= ~ZERO;
    }
    if (c->flags & PLUS)
    {
        c->flags &= ~SPACE;
    }

    /* Parse field width. */
    c->width = 0;
    if (*format == '*')
    {
        format++;
        c->width = va_arg(*args, int);
    }
    else 
    {
        for (; isdigit(*format); format++)
        {
            c->width = c->width * 10 + *format - '0';
        }
    }
    if (c->width < 0) 
    {
        c->width = -c->width;
        c->flags |= MINUS;
    }
      
    /* Parse precision. */
    c->precision = -1;
    if (*format == '.') 
    {
        format++;
        if (*format == '*') 
        {
            format++;
            c->precision = va_arg(*args, int);
        }
        else 
        {
            c->precision = 0;
            for (; isdigit(*format); format++)
            {
                c->precision = c->precision * 10 + *format - '0';
            }
        }
        if (c->precision < 0)
        {
            c->precision = -1;
        }
    }
    if (c->precision >= 0)
    {
        c->flags &= ~ZERO;
    }

    /* Parse type. */
    c->type = INT;
    switch (*format++) 
    {
    case 'h':
    {
        if (*format == 'h') 
        {
            format++;
            c->type = CHAR;
        }
        else
        {
            c->type = SHORT;
        }
        break;
    }
    case 'j':
    {
        c->type = INTMAX;
        break;
    }
    case 'l':
    {
        if (*format == 'l')
        {
            format++;
            c->type = LONGLONG;
        }
        else
        {
            c->type = LONG;
        }
        break;
    }
    case 't':
    {
        c->type = PTRDIFFT;
        break;
    }
    case 'z':
    {
        c->type = SIZET;
        break;
    }
    default:
    {
        format--;
        break;
    }
    }
    return format;    
}

/* Performs an integer conversion, writing output to OUTPUT with
   auxiliary data AUX.  The integer converted has absolute value
   VALUE.  If IS_SIGNED is true, does a signed conversion with
   NEGATIVE indicating a negative value; otherwise does an
   unsigned conversion and ignores NEGATIVE.  The output is done
   according to the provided base B.  Details of the conversion
   are in C. */
static void format_integer(uintmax_t value, bool is_signed, bool negative, 
                           const struct integer_base *b,
                           const struct printf_conversion *c,
                           void (*output)(char, void *), void *aux)
{
    char buf[64], *cp;            /* Buffer and current position. */
    int x;                        /* `x' character to use or 0 if none. */
    int sign;                     /* Sign character or 0 if none. */
    int precision;                /* Rendered precision. */
    int pad_cnt;                  /* # of pad characters to fill field width. */
    int digit_cnt;                /* # of digits output so far. */

    /* Determine sign character, if any.
       An unsigned conversion will never have a sign character,
       even if one of the flags requests one. */
    sign = 0;
    if (is_signed) 
    {
        if (c->flags & PLUS)
        {
            sign = negative ? '-' : '+';
        }
        else if (c->flags & SPACE)
        {
            sign = negative ? '-' : ' ';
        }
        else if (negative)
        {
            sign = '-';
        }
    }

    /* Determine whether to include `0x' or `0X'.
       It will only be included with a hexadecimal conversion of a
       nonzero value with the # flag. */
    x = (c->flags & POUND) && value ? b->x : 0;

    /* Accumulate digits into buffer.
       This algorithm produces digits in reverse order, so later we
       will output the buffer's content in reverse. */
    cp = buf;
    digit_cnt = 0;
    while (value > 0) 
    {
        if ((c->flags & GROUP) && digit_cnt > 0 && digit_cnt % b->group == 0)
        {
            *cp++ = ',';
        }
        *cp++ = b->digits[value % b->base];
        value /= b->base;
        digit_cnt++;
    }

    /* Append enough zeros to match precision.
       If requested precision is 0, then a value of zero is
       rendered as a null string, otherwise as "0".
       If the # flag is used with base 8, the result must always
       begin with a zero. */
    precision = c->precision < 0 ? 1 : c->precision;
    while (cp - buf < precision && cp < buf + sizeof buf - 1)
    {
        *cp++ = '0';
    }
    if ((c->flags & POUND) && b->base == 8 && (cp == buf || cp[-1] != '0'))
    {
        *cp++ = '0';
    }

    /* Calculate number of pad characters to fill field width. */
    pad_cnt = c->width - (cp - buf) - (x ? 2 : 0) - (sign != 0);
    if (pad_cnt < 0)
    {
        pad_cnt = 0;
    }

    /* Do output. */
    if ((c->flags & (MINUS | ZERO)) == 0)
    {
        output_dup(' ', pad_cnt, output, aux);
    }
    if (sign)
    {
        output(sign, aux);
    }
    if (x) 
    {
        output('0', aux);
        output(x, aux); 
    }
    if (c->flags & ZERO)
    {
        output_dup('0', pad_cnt, output, aux);
    }
    while (cp > buf)
    {
        output(*--cp, aux);
    }
    if (c->flags & MINUS)
    {
        output_dup(' ', pad_cnt, output, aux);
    }
}

/* Formats the LENGTH characters starting at STRING according to
   the conversion specified in C.  Writes output to OUTPUT with
   auxiliary data AUX. */
static void format_string(const char *string, int length,
                          struct printf_conversion *c,
                          void (*output)(char, void *), void *aux) 
{
    int i;
    
    if (c->width > length && (c->flags & MINUS) == 0)
    {
        output_dup(' ', c->width - length, output, aux);
    }
    for (i = 0; i < length; i++)
    {
        output(string[i], aux);
    }
    if (c->width > length && (c->flags & MINUS) != 0)
    {
        output_dup(' ', c->width - length, output, aux);
    }
}

static void __vprintf(const char *format, va_list args, void (*output)(char, void *),
                      void *aux)
{
    for (; *format != '\0'; format++)
    {
        struct printf_conversion c;

        /* Literally copy non-conversions to output. */
        if (*format != '%') 
        {
            /* Convert newline to carrige return + newline */
            if (*format == '\n')
            {
                output('\r', aux);
            }
            output(*format, aux);
            continue;
        }
        format++;

        /* %% => %. */
        if (*format == '%') 
        {
            output ('%', aux);
            continue;
        }

        /* Parse conversion specifiers. */
        format = parse_conversion(format, &c, &args);
        
        /* Do conversion. */
        switch (*format) 
        {
        case 'd':
        case 'i': 
        {
            /* Signed integer conversions. */
            intmax_t value;
            
            switch (c.type) 
            {
            case CHAR:
            {
                value = (signed char) va_arg(args, int);
                break;
            }
            case SHORT:
            {
                value = (short) va_arg(args, int);
                break;
            }
            case INT:
            {
                value = va_arg (args, int);
                break;
            }
            case INTMAX:
            {
                value = va_arg (args, intmax_t);
                break;
            }
            case LONG:
            {
                value = va_arg (args, long);
                break;
            }
            case LONGLONG:
            {
                value = va_arg(args, long long);
                break;
            }
            case PTRDIFFT:
            {
                value = va_arg(args, ptrdiff_t);
                break;
            }
            case SIZET:
            {
                value = va_arg(args, size_t);
                if (value > SIZE_MAX / 2)
                {
                    value = value - SIZE_MAX - 1;
                }
                break;
            }
            default:
            {
                break;
            }
            }
            
            format_integer(value < 0 ? -value : value,
                           true, value < 0, &base_d, &c, output, aux);
            break;
        }
        case 'o':
        case 'u':
        case 'x':
        case 'X':
        {
            /* Unsigned integer conversions. */
            uintmax_t value;
            const struct integer_base *b;

            switch (c.type) 
            {
            case CHAR:
            {
                value = (unsigned char) va_arg(args, unsigned);
                break;
            }
            case SHORT:
            {
                value = (unsigned short) va_arg(args, unsigned);
                break;
            }
            case INT:
            {
                value = va_arg(args, unsigned);
                break;
            }
            case INTMAX:
            {
                value = va_arg(args, uintmax_t);
                break;
            }
            case LONG:
            {
                value = va_arg(args, unsigned long);
                break;
            }
            case LONGLONG:
            {
                value = va_arg(args, unsigned long long);
                break;
            }
            case PTRDIFFT:
            {
                value = va_arg(args, ptrdiff_t);
#if UINTMAX_MAX != PTRDIFF_MAX
                value &= ((uintmax_t) PTRDIFF_MAX << 1) | 1;
#endif
                break;
            }
            case SIZET:
            {
                value = va_arg(args, size_t);
                break;
            }
            default:
            {
                break;
            }
            }

            switch (*format) 
            {
            case 'o':
            {
                b = &base_o;
                break;
            }
            case 'u':
            {
                b = &base_d;
                break;
            }
            case 'x':
            {
                b = &base_x;
                break;
            }
            case 'X':
            {
                b = &base_X;
                break;
            }
            default:
            {
                break;
            }
            }

            format_integer(value, false, false, b, &c, output, aux);
            break;
        }
        case 'c': 
        {
            /* Treat character as single-character string. */
            char ch = va_arg(args, int);
            
            format_string (&ch, 1, &c, output, aux);
            break;
        }
        case 's':
        {
            /* String conversion. */
            const char *s = va_arg(args, char *);
            if (s == NULL)
            {
                s = "(null)";
            }

            /* Limit string length according to precision.
               Note: if c.precision == -1 then strnlen() will get
               SIZE_MAX for MAXLEN, which is just what we want. */
            format_string(s, strnlen(s, c.precision), &c, output, aux);
            break;
        }
        case 'p':
        {
            /* Pointer conversion.
               Format pointers as %#x. */
            void *p = va_arg(args, void *);

            c.flags = POUND;
            format_integer((uintptr_t) p, false, false,
                           &base_x, &c, output, aux);
            break;
        }
        case 'f':
        case 'e':
        case 'E':
        case 'g':
        case 'G':
        case 'n':
        {
            /* We don't support floating-point arithmetic,
               and %n can be part of a security hole. */
            __printf("%%%c not supported", output, aux, *format);
            break;
        }
        default:
        {
            __printf("no %%%c conversion>", output, aux, *format);
            break;
        }
        }
    }
}

/* Wrapper for __vprintf() that converts varargs into a
   va_list. */
static void __printf (const char *format,
                      void (*output)(char, void *), void *aux, ...) 
{
    va_list args;

    va_start(args, aux);
    __vprintf(format, args, output, aux);
    va_end(args);
}

/* Writes formatted output to the console. */
int printf(const char *format, ...)
{
    va_list args;
    int retval;

    va_start(args, format);
    retval = vprintf(format, args);
    va_end(args);

    return retval;
}

int vprintf(const char *format, va_list args)
{
    int char_cnt = 0;

    __vprintf(format, args, vprintf_helper, &char_cnt);
    return char_cnt;    
}
