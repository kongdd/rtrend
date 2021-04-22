
<!-- README.md is generated from README.Rmd. Please edit that file -->

# rtrend

<!-- badges: start -->
# <!-- [![R-CMD-check](https://github.com/rpkgs/rtrend/workflows/R-CMD-check/badge.svg)](https://github.com/rpkgs/rtrend/actions) -->
<!-- badges: end -->

## Installation

You can install the released version of rtrend from
[CRAN](https://CRAN.R-project.org) with:

``` r
install.packages("rtrend")
```

## Example

``` r
library(rtrend)
## basic example code
```

``` r
library(rtrend)
set.seed(1)
x <- rnorm(2e2)
microbenchmark::microbenchmark(
    mkTrend(x),
    mkTrend_rcpp(x)
)
#> Unit: microseconds
#>             expr    min      lq     mean  median      uq     max neval
#>       mkTrend(x) 4237.5 4320.60 5830.113 4523.60 5906.45 16492.2   100
#>  mkTrend_rcpp(x)  510.9  541.05  783.561  573.65  717.90  6571.6   100
```

``` r
x <- c(4.81,4.17,4.41,3.59,5.87,3.83, 6.03,4.89,4.32,10,4.69)
par(mar = c(3, 3, 1, 1), mgp = c(2, 0.6, 0))
r_cpp <- mkTrend_rcpp(x, IsPlot = TRUE)
```

<img src="man/figures/README-mkTrend-1.svg" width="100%" />
