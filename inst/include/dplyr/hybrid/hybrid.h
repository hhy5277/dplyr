#ifndef dplyr_hybrid_hybrid_h
#define dplyr_hybrid_hybrid_h

#include <tools/Quosure.h>

#include <dplyr/hybrid/Expression.h>

#include <dplyr/hybrid/scalar_result/n.h>
#include <dplyr/hybrid/scalar_result/sum.h>
#include <dplyr/hybrid/scalar_result/mean_sd_var.h>
#include <dplyr/hybrid/scalar_result/n_distinct.h>
#include <dplyr/hybrid/scalar_result/first_last.h>
#include <dplyr/hybrid/scalar_result/group_indices.h>
#include <dplyr/hybrid/scalar_result/min_max.h>

#include <dplyr/hybrid/vector_result/row_number.h>
#include <dplyr/hybrid/vector_result/ntile.h>
#include <dplyr/hybrid/vector_result/rank.h>
#include <dplyr/hybrid/vector_result/lead_lag.h>
#include <dplyr/hybrid/vector_result/in.h>

#include <dplyr/symbols.h>

namespace dplyr {
namespace hybrid {

#define HYBRID_HANDLE_CASE(__ID__, __FUN__) case __ID__: return __FUN__##_dispatch(data, expression, op);

template <typename SlicedTibble, typename Operation>
SEXP hybrid_do(SEXP expr, const SlicedTibble& data, const DataMask<SlicedTibble>& mask, SEXP env, const Operation& op) {
  if (TYPEOF(expr) != LANGSXP) return R_UnboundValue;

  Expression<SlicedTibble> expression(expr, mask, env);
  switch (expression.get_id()) {
    HYBRID_HANDLE_CASE(N, n)
    HYBRID_HANDLE_CASE(N_DISTINCT, n_distinct)
    HYBRID_HANDLE_CASE(GROUP_INDICES, group_indices)
    HYBRID_HANDLE_CASE(ROW_NUMBER, row_number)
    HYBRID_HANDLE_CASE(SUM, sum)
    HYBRID_HANDLE_CASE(MEAN, mean)
    HYBRID_HANDLE_CASE(VAR, var)
    HYBRID_HANDLE_CASE(SD, sd)
    HYBRID_HANDLE_CASE(FIRST, first)
    HYBRID_HANDLE_CASE(LAST, last)
    HYBRID_HANDLE_CASE(NTH, nth)
    HYBRID_HANDLE_CASE(MIN, min)
    HYBRID_HANDLE_CASE(MAX, max)
    HYBRID_HANDLE_CASE(NTILE, ntile)
    HYBRID_HANDLE_CASE(MIN_RANK, min_rank)
    HYBRID_HANDLE_CASE(DENSE_RANK, dense_rank)
    HYBRID_HANDLE_CASE(PERCENT_RANK, percent_rank)
    HYBRID_HANDLE_CASE(CUME_DIST, cume_dist)
    HYBRID_HANDLE_CASE(LEAD, lead)
    HYBRID_HANDLE_CASE(LAG, lag)
    HYBRID_HANDLE_CASE(IN, in)

  case NOMATCH:
    break;
  }
  return R_UnboundValue;

}

template <typename SlicedTibble>
SEXP summarise(const NamedQuosure& quosure, const SlicedTibble& data, const DataMask<SlicedTibble>& mask) {
  return hybrid_do(quosure.expr(), data, mask, quosure.env(), Summary());
}

template <typename SlicedTibble>
SEXP window(SEXP expr, const SlicedTibble& data, const DataMask<SlicedTibble>& mask, SEXP env) {
  return hybrid_do(expr, data, mask, env, Window());
}

template <typename SlicedTibble>
SEXP match(SEXP expr, const SlicedTibble& data, const DataMask<SlicedTibble>& mask, SEXP env) {
  bool test = !is_vector(expr);
  RObject klass;
  if (test) {
    klass = (hybrid_do(expr, data, mask, env, Match()));
    test = klass != R_UnboundValue;
  }
  LogicalVector res(1, test) ;
  res.attr("class") = "hybrid_call";
  res.attr("call") = expr;
  res.attr("env") = env;

  if (test) {
    Expression<SlicedTibble> expression(expr, mask, env);
    res.attr("fun") = Rf_ScalarString(PRINTNAME(expression.get_fun()));
    res.attr("package") = Rf_ScalarString(PRINTNAME(expression.get_package()));
    res.attr("cpp_class") = klass;

    SEXP call = PROTECT(Rf_duplicate(expr));
    SETCAR(call, Rf_lang3(symbols::double_colon, expression.get_package(), expression.get_fun()));
    res.attr("call") = call;
    UNPROTECT(1);
  }
  return res;
}

}
}

#undef HYBRID_HANDLE_CASE

#endif
