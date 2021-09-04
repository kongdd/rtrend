// #include <Rcpp.h>
#include <RcppArmadillo.h>
using namespace Rcpp;

// [[Rcpp::depends(RcppArmadillo)]]

// This is a simple example of exporting a C++ function to R. You can
// source this function into an R session using the Rcpp::sourceCpp
// function (or via the Source button on the editor toolbar). Learn
// more about Rcpp at:
//
//   http://www.rcpp.org/
//   http://adv-r.had.co.nz/Rcpp.html
//   http://gallery.rcpp.org/
//

// S matrix of Savitzky Golay
// [[Rcpp::export]]
arma::mat sgmat_S(int halfwin = 5, int d = 2) {
    // int res = halfwin + d;
    int frame = 2*halfwin + 1;
    // IntegerVector vec_f = seq(-halfwin, halfwin);
    // IntegerVector vec_d = seq(0, d);

    arma::mat mat = arma::zeros(2*halfwin+1, d+1);

    for (int i = 0; i < frame; i++) {
        for (int j =0; j <= d; j++) {
            mat(i, j) = pow(i * 1.0 - halfwin, j * 1.0);
        }
    }
    // Rcout << mat << std::endl;
    return mat;
}

// B matrix of Savitzky Golay
// [[Rcpp::export]]
arma::mat sgmat_B(const arma::mat S) {
    arma::mat Q, R, B, T;
    // Rcout << repmat(sqrt(w), 1, S.n_cols) % S << std::endl;
    arma::qr_econ(Q, R, S);

    T = solve(R.t(), S.t());
    // Rcout << 'R' <<  R << ;
    B = T.t() * T ;
    return B;
}

// B matrix of weighted Savitzky Golay
// [[Rcpp::export]]
arma::mat sgmat_wB(const arma::mat S, const arma::colvec w) {
    arma::mat Q, R, B, T;
    // Rcout << repmat(sqrt(w), 1, S.n_cols) % S << std::endl;
    arma::qr_econ(Q, R, repmat(sqrt(w), 1, S.n_cols)%S);
    // if (nargout==2) {
    //     mat G = S/(arma::trans(R)*R) ;
    //     B = G*arma::trans(S) ;
    // } else {
        // T = S/R;
        T = solve(R.t(), S.t());
        // Rcout << 'R' <<  R << ;
        B = T.t() * T ;
    // }
    // Rcout << B << std::endl;
    B = repmat(w, 1, B.n_cols).t() % B ;
    return B;
}

//' Weighted Savitzky-Golay
//'
//' NA and Inf values in the y has been ignored automatically.
//'
//' @param y colvec
//' @param w colvec of weight
//' @param halfwin halfwin of Savitzky-Golay
//' @param d polynomial of degree. When d = 1, it becomes moving average.
//'
//' @examples
//' y <- c(1, 3, 2, 5, 6, 8, 10, 1)
//' w <- seq_along(y)/length(y)
//'
//' halfwin = 2
//' d = 2
//' s1 <- smooth_wSG(y, halfwin, d, w)
//' s2 <- smooth_SG(y, halfwin, d)
//' @export
// [[Rcpp::export]]
NumericVector smooth_wSG(
    const arma::colvec y,
    int halfwin=1, int d=1,
    Nullable<NumericVector> w = R_NilValue)
{
    int n = y.size();
    arma::colvec yy(y);
    arma::colvec ww;

    if (w.isNotNull()) {
        ww = as<arma::colvec>(w);
    } else {
        ww = arma::ones<arma::colvec>(n);
    }
    // check Inf and NA
    for (int i = 0; i < n; i++ ) {
        if (!Rcpp::traits::is_finite<REALSXP>(yy[i])){
            ww[i] = 0;
            yy[i] = 0.0; // missing value as zero
        }
    }

    int frame    = halfwin*2 + 1;
    bool is_full = sum(ww) == n;  // no missing value?

    arma::mat S = sgmat_S(halfwin, d);
    arma::mat B = sgmat_wB(S, ww.subvec(0, frame-1));
    arma::colvec y_head = B.rows(0, halfwin) * yy.subvec(0, frame-1);

    arma::colvec y_mid = arma::Col<double>(n-frame-1) ;
    for (int i=1; i<=n-frame-1; i++) {
        if (!is_full) B = sgmat_wB(S, ww.subvec(i, i+frame-1));
        y_mid(i-1) = as_scalar( B.row(halfwin) * yy.subvec(i, i+frame-1) );
    }

    if (!is_full) B = sgmat_wB(S, ww.subvec(n-frame, n-1));
    arma::colvec y_tail = B.rows(halfwin, frame-1) * yy.subvec(n-frame, n-1);

    arma::colvec yfit  = join_vert(y_head, y_mid);
    yfit = join_vert(yfit, y_tail);

    // Rcpp::Rcout << y_head << y_mid << y_tail << std::endl;
    // NumericVector y2 = as<NumericVector>(wrap(yfit));
    // NumericVector(a.begin(),a.end())
    return Rcpp::NumericVector(yfit.begin(), yfit.end());
}

//' @rdname smooth_wSG
//' @export
// [[Rcpp::export]]
Rcpp::NumericVector smooth_SG(const arma::colvec y, const int halfwin=1, const int d=1){
    int n       = y.n_rows;
    int frame   = halfwin*2 + 1;

    arma::mat S = sgmat_S(halfwin, d);
    arma::mat B = sgmat_B(S);
    // Rcpp::Rcout << B << y << std::endl;
    arma::colvec y_head = B.rows(0, halfwin) * y.subvec(0, frame-1);

    arma::colvec y_mid = arma::Col<double>(n-frame-1) ;
    for (int i=1; i<=n-frame-1; i++) {
        y_mid(i-1) = as_scalar( B.row(halfwin) * y.subvec(i, i+frame-1) );
    }

    arma::colvec y_tail = B.rows(halfwin, frame-1) * y.subvec(n-frame, n-1);

    arma::colvec yfit  = join_vert(y_head, y_mid);
    yfit = join_vert(yfit, y_tail);

    return Rcpp::NumericVector(yfit.begin(), yfit.end());
}

//' movmean
//'
//' NA and Inf values in the y will be ignored automatically.
//'
//' @param y A numeric vector.
//' @param halfwin Integer, half of moving window size
//' @param w Corresponding weights of y, with the same length.
//' @param SG_style If true, head and tail values will be in the style of SG
//' (more weights on the center point), else traditional moving mean style.
//'
//' @examples
//' x <- 1:100
//' x[50] <- NA; x[80] <- Inf
//' s1 <- movmean(x, 2, SG_style = TRUE)
//' s2 <- movmean(x, 2, SG_style = FALSE)
//' movmean2(c(4, 8, 6, -1, -2, -3, -1), 2, 0)
//' movmean2(c(4, 8, NA, -1, -2, Inf, -1), 2, 0)
//' @export
// [[Rcpp::export]]
NumericVector movmean(
    const arma::colvec y,
    int halfwin = 1,
    bool SG_style = false,
    Nullable<NumericVector> w = R_NilValue)
{
    int n = y.size();
    arma::colvec yy(y);

    int frame = halfwin*2 + 1;
    int d = 1;
    // Create vector filled with NA(R version)
    arma::colvec ma = yy * NA_REAL; //ma: moving average
    arma::colvec ww = arma::ones<arma::colvec>(n); // weights
    if (w.isNotNull()) {
        ww = as<arma::colvec>(w);
    }
    // check Inf and NA
    for (int i = 0; i < n; i++ ) {
        if (!Rcpp::traits::is_finite<REALSXP>(yy[i])){
            ww[i] = 0;
            yy[i] = 0.0; // missing value as zero
        }
    }

    // main script of moving average
    int i_begin, i_end, n_i;
    double sum, sum_w;
    for (int i = 0; i < n; i++){
        if (i < halfwin) {
            i_begin = 0;
            i_end = i + halfwin;
        } else if (i >= n - halfwin - 1) {
            i_begin = i - halfwin;
            i_end = n-1;
        } else {
            i_begin = i - halfwin;
            i_end = i + halfwin;
        }

        n_i   = 0; // number
        sum   = 0.0; // sum of values in window
        sum_w = 0.0; // sum of weights in window

        for (int j = i_begin; j <= i_end; j++) {
            if (Rcpp::traits::is_finite<REALSXP>(yy[j])) {
                n_i++;
                sum += yy[j];
                sum_w += ww[j];
                // sum += ww[k] * yy[j];
                // sum_wtNA += ww[k];
            }
        }
        if (sum_w > 0) ma[i] = sum/sum_w; // else NA_real_
    }

    if (SG_style) {
        arma::mat S = sgmat_S(halfwin, d);
        arma::mat B = sgmat_wB(S, ww.subvec(0, frame-1));
        // Rcpp::Rcout << B << y << std::endl;
        arma::colvec y_head = B.rows(0, halfwin-1) * yy.subvec(0, frame-1);
        // tail
        B = sgmat_wB(S, ww.subvec(n-frame, n-1));
        arma::colvec y_tail = B.rows(halfwin+1, frame-1) * yy.subvec(n-frame, n-1);

        // Rcout << y_head << y_tail << std::endl;
        for (int i = 0; i < halfwin; i++) {
            ma[i] = y_head[i];
            ma[n - halfwin + i] = y_tail[i];
        }
    }
    return Rcpp::NumericVector(ma.begin(), ma.end());
}

//' @param win_left,win_right windows size in the left and right
//' @rdname movmean
//' @export
// [[Rcpp::export]]
NumericVector movmean2(
    const arma::rowvec &y,
    int win_left = 1,
    int win_right = 0,
    Nullable<NumericVector> w = R_NilValue)
{
    int n = y.size();
    // arma::rowvec yy(y);

    // int frame = halfwin * 2 + 1;
    // int d = 1;
    // Create vector filled with NA(R version)
    arma::rowvec ma = y * NA_REAL;                //ma: moving average
    arma::rowvec ww = arma::ones<arma::rowvec>(n); // weights
    if (w.isNotNull())
    {
        ww = as<arma::colvec>(w);
    }
    // check Inf and NA
    for (int i = 0; i < n; i++)
    {
        if (!Rcpp::traits::is_finite<REALSXP>(y[i]))
        {
            ww[i] = 0;
            // yy[i] = 0.0; // missing value as zero
        }
    }

    // main script of moving average
    int i_begin, i_end, n_i;
    double sum, sum_w;
    for (int i = 0; i < n; i++)
    {
        if (i < win_left) {
            i_begin = 0;
            i_end = i + win_right;
        } else if (i >= n - win_right - 1) {
            i_begin = i - win_left;
            i_end = n - 1;
        } else {
            i_begin = i - win_left;
            i_end = i + win_right;
        }

        n_i = 0;     // number
        sum = 0.0;   // sum of values in window
        sum_w = 0.0; // sum of weights in window

        for (int j = i_begin; j <= i_end; j++)
        {
            if (Rcpp::traits::is_finite<REALSXP>(y[j]))
            {
                n_i++;
                sum += y[j];
                sum_w += ww[j];
                // sum += ww[k] * yy[j];
                // sum_wtNA += ww[k];
            }
        }
        if (sum_w > 0)
            ma[i] = sum / sum_w; // else NA_real_
    }
    // return ma;
    return Rcpp::NumericVector(ma.begin(), ma.end());
}

//' @param mat numeric matrix
//' @rdname movmean
//' @export
// [[Rcpp::export]]
arma::mat movmean_2d(arma::mat& mat, int win_left = 3, int win_right = 0)
{
    int ntime = mat.n_cols;
    int nrow = mat.n_rows;

    arma::mat res = arma::zeros(nrow, ntime);
    // int res = halfwin + d;
    // int frame = 2*halfwin + 1;
    // IntegerVector vec_f = seq(-halfwin, halfwin);
    // IntegerVector vec_d = seq(0, d);

    // arma::colvec
    for (int i = 0; i < nrow; i++)
    {
        res.row(i) = Rcpp::as<arma::rowvec>(movmean2(mat.row(i), win_left, win_right));
    }
    // Rcout << mat << std::endl;
    return res;
}

/*** R
#' @param frame Odd integer

S <- sgmat_S(3, 2)
y <- 1:15
# w <- seq_along(y)/length(y)

frame = 5
d = 2
# smooth_wSG(y, w, frame, d)
# smooth_SG(y, frame, d)

check_perform = FALSE
if (check_perform) {
    x <- 1:100
    x[50] <- NA
    x[80] <- Inf
    halfwin <- 10

    x1 <- movmean(x, halfwin)
    x2 <- movmean(x, 5, 5)

    w <- is.finite(x) %>% as.numeric()

    microbenchmark::microbenchmark(
        movmean(x, halfwin),
        smooth_SG(x, halfwin, 1),
        smooth_wSG(x, halfwin, 1, w)
    )
}
*/
