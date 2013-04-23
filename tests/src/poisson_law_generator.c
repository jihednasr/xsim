#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <poisson_law_generator.h>

/* faire un random entre 0 et 1 */
#define random()    ((double)(rand() % RAND_MAX) / RAND_MAX)

//const unsigned double pi = acos(-1.0);

/* return HUGE_VAL if number too high */
long  poisson_generator_knuth(int lambda)
{
    if (lambda <= 0) 
        return LAMBDA_ERROR;
    long   res   = 0;
    double p     = 1;
    double u     = 0;
    double limit = exp((double) (-1)*lambda);

    do {
        res++;
        do {
            u = random();
        } while (u == 0.0 || u == 1.0);
        p *= u;
    } while (p > limit);

    return (res-1);
}



long poisson_generator_atkinson(int lambda)
{
    if (lambda <= 0)
        return LAMBDA_ERROR;
    double c     = 0.767 - 3.36/lambda;
    double beta  = M_PI/sqrt(3.0*lambda);
    double alpha = beta*lambda;
    double k     = log(c) - lambda - log(beta);

    double u     = 0.0;
    double x     = 0.0;
    int    n     = 0;
    double v     = 0.0;
    double y     = 0.0;
    double lhs   = 0.0;
    double rhs   = 0.0;

    while (1) {
        u = random();
        x = (alpha - log((1.0 - u)/u))/beta;
        n = floor(x + 0.5);
        if (n < 0)
            continue;
        v   = random();
        y   = alpha - beta*x;
        lhs = y + log(v/(1.0 + pow(exp(y), 2)));
        //rhs = k + n*log(lambda) - log(n!);
        rhs = k + n*log(lambda) - lgamma(n+1);
        if (lhs <= rhs)
            return n;
    }
}
