#ifndef __STAN__AGRAD__MATRIX_HPP__
#define __STAN__AGRAD__MATRIX_HPP__

#include <stan/agrad/rev/matrix/fill.hpp>
#include <stan/agrad/rev/matrix/Eigen_NumTraits.hpp>
#include <stan/agrad/rev/matrix/typedefs.hpp>

#include <stan/math/functions/Phi.hpp>
#include <stan/math/functions/logit.hpp>
#include <stan/math/matrix.hpp>
#include <stan/math/matrix_error_handling.hpp>
#include <stan/math/matrix/validate_matching_sizes.hpp>
#include <stan/math/matrix/validate_multiplicable.hpp>
#include <stan/math/matrix/validate_square.hpp>
#include <stan/math/matrix/validate_vector.hpp>


#include <stan/agrad/agrad.hpp>



namespace stan {

  namespace agrad {





    // scalar returns

    // /**
    //  * Determinant of the matrix.
    //  *
    //  * Returns the determinant of the specified
    //  * square matrix.
    //  *
    //  * @param m Specified matrix.
    //  * @return Determinant of the matrix.
    //  * @throw std::domain_error if m is not a square matrix
    //  */
    // var determinant(const Eigen::Matrix<var, Eigen::Dynamic, Eigen::Dynamic>& m);
    
    namespace {
      class dot_self_vari : public vari {
      protected:
        vari** v_;
        size_t size_;
      public:
        dot_self_vari(vari** v, size_t size) 
          : vari(var_dot_self(v,size)), 
            v_(v),
            size_(size) {
        }
        template<typename Derived>
        dot_self_vari(const Eigen::DenseBase<Derived> &v) : 
          vari(var_dot_self(v)), size_(v.size()) {
          v_ = (vari**)memalloc_.alloc(size_*sizeof(vari*));
          for (size_t i = 0; i < size_; i++)
            v_[i] = v[i].vi_;
        }
        template <int R, int C>
        dot_self_vari(const Eigen::Matrix<var,R,C>& v) :
          vari(var_dot_self(v)), size_(v.size()) {
          v_ = (vari**) memalloc_.alloc(size_ * sizeof(vari*));
          for (size_t i = 0; i < size_; ++i)
            v_[i] = v(i).vi_;
        }
        inline static double square(double x) { return x * x; }
        inline static double var_dot_self(vari** v, size_t size) {
          double sum = 0.0;
          for (size_t i = 0; i < size; ++i)
            sum += square(v[i]->val_);
          return sum;
        }
        template<typename Derived>
        double var_dot_self(const Eigen::DenseBase<Derived> &v) {
          double sum = 0.0;
          for (int i = 0; i < v.size(); ++i)
            sum += square(v(i).vi_->val_);
          return sum;
        }
        template <int R, int C>
        inline static double var_dot_self(const Eigen::Matrix<var,R,C> &v) {
          double sum = 0.0;
          for (int i = 0; i < v.size(); ++i)
            sum += square(v(i).vi_->val_);
          return sum;
        }
        virtual void chain() {
          for (size_t i = 0; i < size_; ++i) 
            v_[i]->adj_ += adj_ * 2.0 * v_[i]->val_;
        }
      };

      class sum_v_vari : public vari{
      protected:
        vari** v_;
        size_t length_;
        inline static double var_sum(const var *v, size_t length) {
          double result = 0;
          for (size_t i = 0; i < length; i++)
            result += v[i].vi_->val_;
          return result;
        } 
        template<typename Derived>
        inline static double var_sum(const Eigen::DenseBase<Derived> &v) {
          double result = 0;
          for (int i = 0; i < v.size(); i++)
            result += v(i).vi_->val_;
          return result;
        } 
      public:
        template<typename Derived>
        sum_v_vari(const Eigen::DenseBase<Derived> &v) :
          vari(var_sum(v)), length_(v.size()) {
          v_ = (vari**)memalloc_.alloc(length_*sizeof(vari*));
          for (size_t i = 0; i < length_; i++)
            v_[i] = v(i).vi_;
        }
        template<int R1,int C1>
        sum_v_vari(const Eigen::Matrix<var,R1,C1> &v1) :
          vari(var_sum(v1)), length_(v1.size()) {
          v_ = (vari**)memalloc_.alloc(length_*sizeof(vari*));
          for (size_t i = 0; i < length_; i++)
            v_[i] = v1(i).vi_;
        }
        sum_v_vari(const var *v, size_t len) :
          vari(var_sum(v,len)), length_(len) {
          v_ = (vari**)memalloc_.alloc(length_*sizeof(vari*));
          for (size_t i = 0; i < length_; i++)
            v_[i] = v[i].vi_;
        }
        virtual void chain() {
          for (size_t i = 0; i < length_; i++) {
            v_[i]->adj_ += adj_;
          }
        }
      };
      class dot_product_vv_vari : public vari {
      protected:
        vari** v1_;
        vari** v2_;
        size_t length_;
        inline static double var_dot(const var* v1, const var* v2,
                                     size_t length) {
          double result = 0;
          for (size_t i = 0; i < length; i++)
            result += v1[i].vi_->val_ * v2[i].vi_->val_;
          return result;
        }
        template<typename Derived1,typename Derived2>
        inline static double var_dot(const Eigen::DenseBase<Derived1> &v1,
                                     const Eigen::DenseBase<Derived2> &v2) {
          double result = 0;
          for (int i = 0; i < v1.size(); i++)
            result += v1[i].vi_->val_ * v2[i].vi_->val_;
          return result;
        }
        inline static double var_dot(vari** v1, vari** v2, size_t length) {
          double result = 0;
          for (size_t i = 0; i < length; ++i)
            result += v1[i]->val_ * v2[i]->val_;
          return result;
        }
      public:
        dot_product_vv_vari(vari** v1, vari** v2, size_t length)
          : vari(var_dot(v1,v2,length)),
            v1_(v1), 
            v2_(v2), 
            length_(length) {

        }
        dot_product_vv_vari(const var* v1, const var* v2, size_t length,
                            dot_product_vv_vari* shared_v1 = NULL,
                            dot_product_vv_vari* shared_v2 = NULL) : 
          vari(var_dot(v1, v2, length)), length_(length) {
          if (shared_v1 == NULL) {
            v1_ = (vari**)memalloc_.alloc(length_*sizeof(vari*));
            for (size_t i = 0; i < length_; i++)
              v1_[i] = v1[i].vi_;
          }
          else {
            v1_ = shared_v1->v1_;
          }
          if (shared_v2 == NULL) {
            v2_ = (vari**)memalloc_.alloc(length_*sizeof(vari*));
            for (size_t i = 0; i < length_; i++)
              v2_[i] = v2[i].vi_;
          }
          else {
            v2_ = shared_v2->v2_;
          }
        }
        template<typename Derived1,typename Derived2>
        dot_product_vv_vari(const Eigen::DenseBase<Derived1> &v1,
                            const Eigen::DenseBase<Derived2> &v2,
                            dot_product_vv_vari* shared_v1 = NULL,
                            dot_product_vv_vari* shared_v2 = NULL) : 
          vari(var_dot(v1, v2)), length_(v1.size()) {
          if (shared_v1 == NULL) {
            v1_ = (vari**)memalloc_.alloc(length_*sizeof(vari*));
            for (size_t i = 0; i < length_; i++)
              v1_[i] = v1[i].vi_;
          }
          else {
            v1_ = shared_v1->v1_;
          }
          if (shared_v2 == NULL) {
            v2_ = (vari**)memalloc_.alloc(length_*sizeof(vari*));
            for (size_t i = 0; i < length_; i++)
              v2_[i] = v2[i].vi_;
          }
          else {
            v2_ = shared_v2->v2_;
          }
        }
        template<int R1,int C1,int R2,int C2>
        dot_product_vv_vari(const Eigen::Matrix<var,R1,C1> &v1,
                            const Eigen::Matrix<var,R2,C2> &v2,
                            dot_product_vv_vari* shared_v1 = NULL,
                            dot_product_vv_vari* shared_v2 = NULL) : 
          vari(var_dot(v1, v2)), length_(v1.size()) {
          if (shared_v1 == NULL) {
            v1_ = (vari**)memalloc_.alloc(length_*sizeof(vari*));
            for (size_t i = 0; i < length_; i++)
              v1_[i] = v1[i].vi_;
          }
          else {
            v1_ = shared_v1->v1_;
          }
          if (shared_v2 == NULL) {
            v2_ = (vari**)memalloc_.alloc(length_*sizeof(vari*));
            for (size_t i = 0; i < length_; i++)
              v2_[i] = v2[i].vi_;
          }
          else {
            v2_ = shared_v2->v2_;
          }
        }
        virtual void chain() {
          for (size_t i = 0; i < length_; i++) {
            v1_[i]->adj_ += adj_ * v2_[i]->val_;
            v2_[i]->adj_ += adj_ * v1_[i]->val_;
          }
        }
      };

      class dot_product_vd_vari : public vari {
      protected:
        vari** v1_;
        double* v2_;
        size_t length_;
        inline static double var_dot(const var* v1, const double* v2,
                                     size_t length) {
          double result = 0;
          for (size_t i = 0; i < length; i++)
            result += v1[i].vi_->val_ * v2[i];
          return result;
        }
        template<typename Derived1,typename Derived2>
        inline static double var_dot(const Eigen::DenseBase<Derived1> &v1,
                                     const Eigen::DenseBase<Derived2> &v2) {
          double result = 0;
          for (int i = 0; i < v1.size(); i++)
            result += v1[i].vi_->val_ * v2[i];
          return result;
        }
      public:
        dot_product_vd_vari(const var* v1, const double* v2, size_t length,
                            dot_product_vd_vari *shared_v1 = NULL,
                            dot_product_vd_vari *shared_v2 = NULL) : 
          vari(var_dot(v1, v2, length)), length_(length) {
          if (shared_v1 == NULL) {
            v1_ = (vari**)memalloc_.alloc(length_*sizeof(vari*));
            for (size_t i = 0; i < length_; i++)
              v1_[i] = v1[i].vi_;
          } else {
            v1_ = shared_v1->v1_;
          }
          if (shared_v2 == NULL) {
            v2_ = (double*)memalloc_.alloc(length_*sizeof(double));
            for (size_t i = 0; i < length_; i++)
              v2_[i] = v2[i];
          } else {
            v2_ = shared_v2->v2_;
          }
        }
        template<typename Derived1,typename Derived2>
        dot_product_vd_vari(const Eigen::DenseBase<Derived1> &v1,
                            const Eigen::DenseBase<Derived2> &v2,
                            dot_product_vd_vari *shared_v1 = NULL,
                            dot_product_vd_vari *shared_v2 = NULL) : 
          vari(var_dot(v1, v2)), length_(v1.size()) {
          if (shared_v1 == NULL) {
            v1_ = (vari**)memalloc_.alloc(length_*sizeof(vari*));
            for (size_t i = 0; i < length_; i++)
              v1_[i] = v1[i].vi_;
          } else {
            v1_ = shared_v1->v1_;
          }
          if (shared_v2 == NULL) {
            v2_ = (double*)memalloc_.alloc(length_*sizeof(double));
            for (size_t i = 0; i < length_; i++)
              v2_[i] = v2[i];
          } else {
            v2_ = shared_v2->v2_;
          }
        }
        template<int R1,int C1,int R2,int C2>
        dot_product_vd_vari(const Eigen::Matrix<var,R1,C1> &v1,
                            const Eigen::Matrix<double,R2,C2> &v2,
                            dot_product_vd_vari *shared_v1 = NULL,
                            dot_product_vd_vari *shared_v2 = NULL) : 
          vari(var_dot(v1, v2)), length_(v1.size()) {
          if (shared_v1 == NULL) {
            v1_ = (vari**)memalloc_.alloc(length_*sizeof(vari*));
            for (size_t i = 0; i < length_; i++)
              v1_[i] = v1[i].vi_;
          } else {
            v1_ = shared_v1->v1_;
          }
          if (shared_v2 == NULL) {
            v2_ = (double*)memalloc_.alloc(length_*sizeof(double));
            for (size_t i = 0; i < length_; i++)
              v2_[i] = v2[i];
          } else {
            v2_ = shared_v2->v2_;
          }
        }
        virtual void chain() {
          for (size_t i = 0; i < length_; i++) {
            v1_[i]->adj_ += adj_ * v2_[i];
          }
        }
      };

    }

    /**
     * Returns the dot product of a vector with itself.
     *
     * @param[in] v Vector.
     * @return Dot product of the vector with itself.
     * @tparam R number of rows or <code>Eigen::Dynamic</code> for
     * dynamic; one of R or C must be 1
     * @tparam C number of rows or <code>Eigen::Dyanmic</code> for
     * dynamic; one of R or C must be 1
     */
    template<int R, int C>
    inline var dot_self(const Eigen::Matrix<var, R, C>& v) {
      stan::math::validate_vector(v,"dot_self");
      return var(new dot_self_vari(v));
    }
    
    /**
     * Returns the dot product of each column of a matrix with itself.
     * @param x Matrix.
     * @tparam T scalar type
     */
    template<int R,int C>
    inline Eigen::Matrix<var,1,C> 
    columns_dot_self(const Eigen::Matrix<var,R,C>& x) {
      Eigen::Matrix<var,1,C> ret(1,x.cols());
      for (size_type i = 0; i < x.cols(); i++) {
        ret(i) = var(new dot_self_vari(x.col(i)));
      }
      return ret;
    }
    
    template<int R1,int C1,int R2, int C2>
    inline Eigen::Matrix<var, 1, C1>
    columns_dot_product(const Eigen::Matrix<var, R1, C1>& v1, 
                        const Eigen::Matrix<var, R2, C2>& v2) {
      stan::math::validate_matching_sizes(v1,v2,"columns_dot_product");
      Eigen::Matrix<var, 1, C1> ret(1,v1.cols());
      for (size_type j = 0; j < v1.cols(); ++j) {
        ret(j) = var(new dot_product_vv_vari(v1.col(j),v2.col(j)));
      }
      return ret;
    }
    
    template<int R1,int C1,int R2, int C2>
    inline Eigen::Matrix<var, 1, C1>
    columns_dot_product(const Eigen::Matrix<var, R1, C1>& v1, 
                        const Eigen::Matrix<double, R2, C2>& v2) {
      stan::math::validate_matching_sizes(v1,v2,"columns_dot_product");
      Eigen::Matrix<var, 1, C1> ret(1,v1.cols());
      size_t j;
      for (j = 0; j < v1.cols(); ++j) {
        ret(j) = var(new dot_product_vd_vari(v1.col(j),v2.col(j)));
      }
      return ret;
    }

    template<int R1,int C1,int R2, int C2>
    inline Eigen::Matrix<var, 1, C1>
    columns_dot_product(const Eigen::Matrix<double, R1, C1>& v1, 
                        const Eigen::Matrix<var, R2, C2>& v2) {
      stan::math::validate_matching_sizes(v1,v2,"columns_dot_product");
      Eigen::Matrix<var, 1, C1> ret(1,v1.cols());
      for (size_type j = 0; j < v1.cols(); ++j) {
        ret(j) = var(new dot_product_vd_vari(v2.col(j),v1.col(j)));
      }
      return ret;
    }
    
    /**
     * Returns the dot product.
     *
     * @param[in] v1 First column vector.
     * @param[in] v2 Second column vector.
     * @return Dot product of the vectors.
     * @throw std::domain_error if length of v1 is not equal to length of v2.
     */
    template<int R1,int C1,int R2, int C2>
    inline var dot_product(const Eigen::Matrix<var, R1, C1>& v1, 
                           const Eigen::Matrix<var, R2, C2>& v2) {
      stan::math::validate_vector(v1,"dot_product");
      stan::math::validate_vector(v2,"dot_product");
      stan::math::validate_matching_sizes(v1,v2,"dot_product");
      return var(new dot_product_vv_vari(v1,v2));
    }
    /**
     * Returns the dot product.
     *
     * @param[in] v1 First column vector.
     * @param[in] v2 Second column vector.
     * @return Dot product of the vectors.
     * @throw std::domain_error if length of v1 is not equal to length of v2
     * or either v1 or v2 are not vectors.
     */
    template<int R1,int C1,int R2, int C2>
    inline var dot_product(const Eigen::Matrix<var, R1, C1>& v1, 
                           const Eigen::Matrix<double, R2, C2>& v2) {
      stan::math::validate_vector(v1,"dot_product");
      stan::math::validate_vector(v2,"dot_product");
      stan::math::validate_matching_sizes(v1,v2,"dot_product");
      return var(new dot_product_vd_vari(v1,v2));
    }
    /**
     * Returns the dot product.
     *
     * @param[in] v1 First column vector.
     * @param[in] v2 Second column vector.
     * @return Dot product of the vectors.
     * @throw std::domain_error if length of v1 is not equal to length of v2
     * or either v1 or v2 are not vectors.
     */
    template<int R1,int C1,int R2, int C2>
    inline var dot_product(const Eigen::Matrix<double, R1, C1>& v1, 
                           const Eigen::Matrix<var, R2, C2>& v2) {
      stan::math::validate_vector(v1,"dot_product");
      stan::math::validate_vector(v2,"dot_product");
      stan::math::validate_matching_sizes(v1,v2,"dot_product");
      return var(new dot_product_vd_vari(v2,v1));
    }
    /**
     * Returns the dot product.
     *
     * @param[in] v1 First array.
     * @param[in] v2 Second array.
     * @param[in] length Length of both arrays.
     * @return Dot product of the arrays.
     */
    inline var dot_product(const var* v1, const var* v2, size_t length) {
      return var(new dot_product_vv_vari(v1, v2, length));
    }
    /**
     * Returns the dot product.
     *
     * @param[in] v1 First array.
     * @param[in] v2 Second array.
     * @param[in] length Length of both arrays.
     * @return Dot product of the arrays.
     */
    inline var dot_product(const var* v1, const double* v2, size_t length) {
      return var(new dot_product_vd_vari(v1, v2, length));
    }
    /**
     * Returns the dot product.
     *
     * @param[in] v1 First array.
     * @param[in] v2 Second array.
     * @param[in] length Length of both arrays.
     * @return Dot product of the arrays.
     */
    inline var dot_product(const double* v1, const var* v2, size_t length) {
      return var(new dot_product_vd_vari(v2, v1, length));
    }
    /**
     * Returns the dot product.
     *
     * @param[in] v1 First vector.
     * @param[in] v2 Second vector.
     * @return Dot product of the vectors.
     * @throw std::domain_error if sizes of v1 and v2 do not match.
     */
    inline var dot_product(const std::vector<var>& v1,
                           const std::vector<var>& v2) {
      stan::math::validate_matching_sizes(v1,v2,"dot_product");
      return var(new dot_product_vv_vari(&v1[0], &v2[0], v1.size()));
    }
    /**
     * Returns the dot product.
     *
     * @param[in] v1 First vector.
     * @param[in] v2 Second vector.
     * @return Dot product of the vectors.
     * @throw std::domain_error if sizes of v1 and v2 do not match.
     */
    inline var dot_product(const std::vector<var>& v1,
                           const std::vector<double>& v2) {
      stan::math::validate_matching_sizes(v1,v2,"dot_product");
      return var(new dot_product_vd_vari(&v1[0], &v2[0], v1.size()));
    }
    /**
     * Returns the dot product.
     *
     * @param[in] v1 First vector.
     * @param[in] v2 Second vector.
     * @return Dot product of the vectors.
     * @throw std::domain_error if sizes of v1 and v2 do not match.
     */
    inline var dot_product(const std::vector<double>& v1,
                           const std::vector<var>& v2) {
      stan::math::validate_matching_sizes(v1,v2,"dot_product");
      return var(new dot_product_vd_vari(&v2[0], &v1[0], v1.size()));
    }

    /**
     * Returns the sum of the coefficients of the specified
     * matrix, column vector or row vector.
     * @param m Specified matrix or vector.
     * @return Sum of coefficients of matrix.
     */
    template <int R, int C>
    inline var sum(const Eigen::Matrix<var,R,C>& m) {
      if (m.size() == 0)
        return 0.0;
      return var(new sum_v_vari(m));
    }

    template <int R1,int C1,int R2,int C2>
    class mdivide_left_vv_vari : public vari {
    public:
      int _M; // A.rows() = A.cols() = B.rows()
      int _N; // B.cols()
      double* _A;
      double* _C;
      vari** _variRefA;
      vari** _variRefB;
      vari** _variRefC;

      mdivide_left_vv_vari(const Eigen::Matrix<var,R1,C1> &A,
                           const Eigen::Matrix<var,R2,C2> &B)
      : vari(0.0),
   _M(A.rows()),
  _N(B.cols()),
  _A((double*)stan::agrad::memalloc_.alloc(sizeof(double) 
                                                 * A.rows() * A.cols())),
  _C((double*)stan::agrad::memalloc_.alloc(sizeof(double) 
                                                 * B.rows() * B.cols())),
        _variRefA((vari**)stan::agrad::memalloc_.alloc(sizeof(vari*) 
                   * A.rows() * A.cols())),
        _variRefB((vari**)stan::agrad::memalloc_.alloc(sizeof(vari*) 
                   * B.rows() * B.cols())),
        _variRefC((vari**)stan::agrad::memalloc_.alloc(sizeof(vari*) 
                                                      * B.rows() * B.cols()))
      {
  using Eigen::Matrix;
        using Eigen::Map;

  size_t pos = 0;
  for (size_type j = 0; j < _M; j++) {
          for (size_type i = 0; i < _M; i++) {
            _variRefA[pos] = A(i,j).vi_;
      _A[pos++] = A(i,j).val();
          }
        }
  
  pos = 0;
  for (size_type j = 0; j < _N; j++) {
          for (size_type i = 0; i < _M; i++) {
            _variRefB[pos] = B(i,j).vi_;
            _C[pos++] = B(i,j).val();
          }
        }
        
  Matrix<double,R1,C2> C(_M,_N);
  C = Map<Matrix<double,R1,C2> >(_C,_M,_N);

  C = Map<Matrix<double,R1,C1> >(_A,_M,_M)
    .colPivHouseholderQr().solve(C);

  pos = 0;
        for (size_type j = 0; j < _N; j++) {
          for (size_type i = 0; i < _M; i++) {
      _C[pos] = C(i,j);
            _variRefC[pos] = new vari(_C[pos],false);
      pos++;
          }
        }
      }
      
      virtual void chain() {
  using Eigen::Matrix;
        using Eigen::Map;
  Eigen::Matrix<double,R1,C1> adjA(_M,_M);
        Eigen::Matrix<double,R2,C2> adjB(_M,_N);
        Eigen::Matrix<double,R1,C2> adjC(_M,_N);

  size_t pos = 0;
        for (size_type j = 0; j < adjC.cols(); j++)
          for (size_type i = 0; i < adjC.rows(); i++)
            adjC(i,j) = _variRefC[pos++]->adj_;
        
        
  adjB = Map<Matrix<double,R1,C1> >(_A,_M,_M)
    .transpose().colPivHouseholderQr().solve(adjC);
  adjA.noalias() = -adjB
    * Map<Matrix<double,R1,C2> >(_C,_M,_N).transpose();
        
  pos = 0;
        for (size_type j = 0; j < adjA.cols(); j++)
          for (size_type i = 0; i < adjA.rows(); i++)
            _variRefA[pos++]->adj_ += adjA(i,j);
        
  pos = 0;
        for (size_type j = 0; j < adjB.cols(); j++)
          for (size_type i = 0; i < adjB.rows(); i++)
            _variRefB[pos++]->adj_ += adjB(i,j);
      }
    };
    
    template <int R1,int C1,int R2,int C2>
    class mdivide_left_dv_vari : public vari {
    public:
      int _M; // A.rows() = A.cols() = B.rows()
      int _N; // B.cols()
      double* _A;
      double* _C;
      vari** _variRefB;
      vari** _variRefC;
      
      mdivide_left_dv_vari(const Eigen::Matrix<double,R1,C1> &A,
                           const Eigen::Matrix<var,R2,C2> &B)
      : vari(0.0),
   _M(A.rows()),
  _N(B.cols()),
  _A((double*)stan::agrad::memalloc_.alloc(sizeof(double) 
                                                 * A.rows() * A.cols())),
  _C((double*)stan::agrad::memalloc_.alloc(sizeof(double) 
                                                 * B.rows() * B.cols())),
        _variRefB((vari**)stan::agrad::memalloc_.alloc(sizeof(vari*) 
                   * B.rows() * B.cols())),
        _variRefC((vari**)stan::agrad::memalloc_.alloc(sizeof(vari*) 
                   * B.rows() * B.cols()))
      {
  using Eigen::Matrix;
  using Eigen::Map;
  
  size_t pos = 0;
  for (size_type j = 0; j < _M; j++) {
    for (size_type i = 0; i < _M; i++) {
      _A[pos++] = A(i,j);
    }
  }
  
  pos = 0;
  for (size_type j = 0; j < _N; j++) {
          for (size_type i = 0; i < _M; i++) {
            _variRefB[pos] = B(i,j).vi_;
            _C[pos++] = B(i,j).val();
          }
        }
                
  Matrix<double,R1,C2> C(_M,_N);
  C = Map<Matrix<double,R1,C2> >(_C,_M,_N);

  C = Map<Matrix<double,R1,C1> >(_A,_M,_M)
    .colPivHouseholderQr().solve(C);
  
  pos = 0;
        for (size_type j = 0; j < _N; j++) {
          for (size_type i = 0; i < _M; i++) {
      _C[pos] = C(i,j);
            _variRefC[pos] = new vari(_C[pos],false);
      pos++;
          }
        }
      }
      
      virtual void chain() {
  using Eigen::Matrix;
        using Eigen::Map;
        Eigen::Matrix<double,R2,C2> adjB(_M,_N);
        Eigen::Matrix<double,R1,C2> adjC(_M,_N);

  size_t pos = 0;
        for (size_type j = 0; j < adjC.cols(); j++)
          for (size_type i = 0; i < adjC.rows(); i++)
            adjC(i,j) = _variRefC[pos++]->adj_;

        adjB = Map<Matrix<double,R1,C1> >(_A,_M,_M)
    .transpose().colPivHouseholderQr().solve(adjC);

  pos = 0;
        for (size_type j = 0; j < adjB.cols(); j++)
          for (size_type i = 0; i < adjB.rows(); i++)
            _variRefB[pos++]->adj_ += adjB(i,j);
      }
    };
    
    template <int R1,int C1,int R2,int C2>
    class mdivide_left_vd_vari : public vari {
    public:
      int _M; // A.rows() = A.cols() = B.rows()
      int _N; // B.cols()
      double* _A;
      double* _C;
      vari** _variRefA;
      vari** _variRefC;
      
      mdivide_left_vd_vari(const Eigen::Matrix<var,R1,C1> &A,
                           const Eigen::Matrix<double,R2,C2> &B)
      : vari(0.0),
   _M(A.rows()),
  _N(B.cols()),
  _A((double*)stan::agrad::memalloc_.alloc(sizeof(double) 
                                                 * A.rows() * A.cols())),
  _C((double*)stan::agrad::memalloc_.alloc(sizeof(double) 
                                                 * B.rows() * B.cols())),
        _variRefA((vari**)stan::agrad::memalloc_.alloc(sizeof(vari*) 
                   * A.rows() * A.cols())),
        _variRefC((vari**)stan::agrad::memalloc_.alloc(sizeof(vari*) 
                                                      * B.rows() * B.cols()))
      {
  using Eigen::Matrix;
        using Eigen::Map;

  size_t pos = 0;
  for (size_type j = 0; j < _M; j++) {
          for (size_type i = 0; i < _M; i++) {
            _variRefA[pos] = A(i,j).vi_;
      _A[pos++] = A(i,j).val();
          }
        }
  
        Matrix<double,R1,C2> C(_M,_N);
  C = Map<Matrix<double,R1,C1> >(_A,_M,_M)
    .colPivHouseholderQr().solve(B);

  pos = 0;
        for (size_type j = 0; j < _N; j++) {
          for (size_type i = 0; i < _M; i++) {
      _C[pos] = C(i,j);
            _variRefC[pos] = new vari(_C[pos],false);
      pos++;
          }
        }
      }
      
      virtual void chain() {
  using Eigen::Matrix;
        using Eigen::Map;
  Eigen::Matrix<double,R1,C1> adjA(_M,_M);
        Eigen::Matrix<double,R1,C2> adjC(_M,_N);

  size_t pos = 0;
        for (size_type j = 0; j < adjC.cols(); j++)
          for (size_type i = 0; i < adjC.rows(); i++)
            adjC(i,j) = _variRefC[pos++]->adj_;
        
  // FIXME: add .noalias() to LHS
  adjA = -Map<Matrix<double,R1,C1> >(_A,_M,_M)
    .transpose()
    .colPivHouseholderQr()
    .solve(adjC*Map<Matrix<double,R1,C2> >(_C,_M,_N).transpose());

  pos = 0;
        for (size_type j = 0; j < adjA.cols(); j++)
          for (size_type i = 0; i < adjA.rows(); i++)
            _variRefA[pos++]->adj_ += adjA(i,j);
      }
    };

    template <int R1,int C1,int R2,int C2>
    inline 
    Eigen::Matrix<var,R1,C2>
    mdivide_left(const Eigen::Matrix<var,R1,C1> &A,
                 const Eigen::Matrix<var,R2,C2> &b) {
      Eigen::Matrix<var,R1,C2> res(b.rows(),b.cols());
      
      stan::math::validate_square(A,"mdivide_left");
      stan::math::validate_multiplicable(A,b,"mdivide_left");
      
      // NOTE: this is not a memory leak, this vari is used in the 
      // expression graph to evaluate the adjoint, but is not needed
      // for the returned matrix.  Memory will be cleaned up with the arena allocator.
      mdivide_left_vv_vari<R1,C1,R2,C2> *baseVari = new mdivide_left_vv_vari<R1,C1,R2,C2>(A,b);
      
      size_t pos = 0;
      for (size_type j = 0; j < res.cols(); j++)
  for (size_type i = 0; i < res.rows(); i++)
          res(i,j).vi_ = baseVari->_variRefC[pos++];
      
      return res;
    }

    template <int R1,int C1,int R2,int C2>
    inline 
    Eigen::Matrix<var,R1,C2>
    mdivide_left(const Eigen::Matrix<var,R1,C1> &A,
                 const Eigen::Matrix<double,R2,C2> &b) {
      Eigen::Matrix<var,R1,C2> res(b.rows(),b.cols());
      
      stan::math::validate_square(A,"mdivide_left");
      stan::math::validate_multiplicable(A,b,"mdivide_left");
      
      // NOTE: this is not a memory leak, this vari is used in the 
      // expression graph to evaluate the adjoint, but is not needed
      // for the returned matrix.  Memory will be cleaned up with the arena allocator.
      mdivide_left_vd_vari<R1,C1,R2,C2> *baseVari = new mdivide_left_vd_vari<R1,C1,R2,C2>(A,b);
      
      size_t pos = 0;
      for (size_type j = 0; j < res.cols(); j++)
  for (size_type i = 0; i < res.rows(); i++)
          res(i,j).vi_ = baseVari->_variRefC[pos++];
      
      return res;
    }
    
    template <int R1,int C1,int R2,int C2>
    inline 
    Eigen::Matrix<var,R1,C2>
    mdivide_left(const Eigen::Matrix<double,R1,C1> &A,
                 const Eigen::Matrix<var,R2,C2> &b) {
      Eigen::Matrix<var,R1,C2> res(b.rows(),b.cols());
      
      stan::math::validate_square(A,"mdivide_left");
      stan::math::validate_multiplicable(A,b,"mdivide_left");
      
      // NOTE: this is not a memory leak, this vari is used in the 
      // expression graph to evaluate the adjoint, but is not needed
      // for the returned matrix.  Memory will be cleaned up with the arena allocator.
      mdivide_left_dv_vari<R1,C1,R2,C2> *baseVari = new mdivide_left_dv_vari<R1,C1,R2,C2>(A,b);
      
      size_t pos = 0;
      for (size_type j = 0; j < res.cols(); j++)
  for (size_type i = 0; i < res.rows(); i++)
          res(i,j).vi_ = baseVari->_variRefC[pos++];
      
      return res;
    }
    
    template <int TriView,int R1,int C1,int R2,int C2>
    class mdivide_left_tri_vv_vari : public vari {
    public:
      int _M; // A.rows() = A.cols() = B.rows()
      int _N; // B.cols()
      double* _A;
      double* _C;
      vari** _variRefA;
      vari** _variRefB;
      vari** _variRefC;
      
      mdivide_left_tri_vv_vari(const Eigen::Matrix<var,R1,C1> &A,
                               const Eigen::Matrix<var,R2,C2> &B)
      : vari(0.0),
  _M(A.rows()),
  _N(B.cols()),
  _A((double*)stan::agrad::memalloc_.alloc(sizeof(double) 
                                                 * A.rows() * A.cols())),
  _C((double*)stan::agrad::memalloc_.alloc(sizeof(double) 
                                                 * B.rows() * B.cols())),
        _variRefA((vari**)stan::agrad::memalloc_.alloc(sizeof(vari*) 
                   * A.rows() 
                   * (A.rows() + 1) / 2)),
        _variRefB((vari**)stan::agrad::memalloc_.alloc(sizeof(vari*) 
                   * B.rows() * B.cols())),
        _variRefC((vari**)stan::agrad::memalloc_.alloc(sizeof(vari*) 
                                                      * B.rows() * B.cols()))
      {
  using Eigen::Matrix;
        using Eigen::Map;

  size_t pos = 0;
  if (TriView == Eigen::Lower) {
    for (size_type j = 0; j < _M; j++)
      for (size_type i = j; i < _M; i++)
        _variRefA[pos++] = A(i,j).vi_;
  } else if (TriView == Eigen::Upper) {
    for (size_type j = 0; j < _M; j++)
      for (size_type i = 0; i < j+1; i++)
        _variRefA[pos++] = A(i,j).vi_;
  }

  pos = 0;
  for (size_type j = 0; j < _M; j++) {
          for (size_type i = 0; i < _M; i++) {
      _A[pos++] = A(i,j).val();
          }
        }
  
  pos = 0;
  for (size_type j = 0; j < _N; j++) {
          for (size_type i = 0; i < _M; i++) {
            _variRefB[pos] = B(i,j).vi_;
            _C[pos++] = B(i,j).val();
          }
        }
        
  Matrix<double,R1,C2> C(_M,_N);
  C = Map<Matrix<double,R1,C2> >(_C,_M,_N);

  C = Map<Matrix<double,R1,C1> >(_A,_M,_M)
    .template triangularView<TriView>().solve(C);

  pos = 0;
        for (size_type j = 0; j < _N; j++) {
          for (size_type i = 0; i < _M; i++) {
      _C[pos] = C(i,j);
            _variRefC[pos] = new vari(_C[pos],false);
      pos++;
          }
        }

      }
      
      virtual void chain() {
  using Eigen::Matrix;
        using Eigen::Map;
  Matrix<double,R1,C1> adjA(_M,_M);
        Matrix<double,R2,C2> adjB(_M,_N);
        Matrix<double,R1,C2> adjC(_M,_N);

  size_t pos = 0;
        for (size_type j = 0; j < adjC.cols(); j++)
          for (size_type i = 0; i < adjC.rows(); i++)
            adjC(i,j) = _variRefC[pos++]->adj_;
        
  adjB = Map<Matrix<double,R1,C1> >(_A,_M,_M)
    .template triangularView<TriView>().transpose().solve(adjC);
  adjA.noalias() = -adjB
    * Map<Matrix<double,R1,C2> >(_C,_M,_N).transpose();
        
  pos = 0;
  if (TriView == Eigen::Lower) {
    for (size_type j = 0; j < adjA.cols(); j++)
      for (size_type i = j; i < adjA.rows(); i++)
        _variRefA[pos++]->adj_ += adjA(i,j);
  } else if (TriView == Eigen::Upper) {
    for (size_type j = 0; j < adjA.cols(); j++)
      for (size_type i = 0; i < j+1; i++)
        _variRefA[pos++]->adj_ += adjA(i,j);
  } 
        
  pos = 0;
        for (size_type j = 0; j < adjB.cols(); j++)
          for (size_type i = 0; i < adjB.rows(); i++)
            _variRefB[pos++]->adj_ += adjB(i,j);
      }
    };

    template <int TriView,int R1,int C1,int R2,int C2>
    class mdivide_left_tri_dv_vari : public vari {
    public:
      int _M; // A.rows() = A.cols() = B.rows()
      int _N; // B.cols()
      double* _A;
      double* _C;
      vari** _variRefB;
      vari** _variRefC;
      
      mdivide_left_tri_dv_vari(const Eigen::Matrix<double,R1,C1> &A,
                               const Eigen::Matrix<var,R2,C2> &B)
      : vari(0.0),
  _M(A.rows()),
  _N(B.cols()),
  _A((double*)stan::agrad::memalloc_.alloc(sizeof(double) 
                                                 * A.rows() * A.cols())),
  _C((double*)stan::agrad::memalloc_.alloc(sizeof(double) 
                                                 * B.rows() * B.cols())),
        _variRefB((vari**)stan::agrad::memalloc_.alloc(sizeof(vari*) 
                   * B.rows() * B.cols())),
        _variRefC((vari**)stan::agrad::memalloc_.alloc(sizeof(vari*) 
                                                      * B.rows() * B.cols()))
      {
  using Eigen::Matrix;
        using Eigen::Map;

  size_t pos = 0;
  for (size_type j = 0; j < _M; j++) {
          for (size_type i = 0; i < _M; i++) {
      _A[pos++] = A(i,j);
          }
        }

  pos = 0;
  for (size_type j = 0; j < _N; j++) {
          for (size_type i = 0; i < _M; i++) {
            _variRefB[pos] = B(i,j).vi_;
            _C[pos++] = B(i,j).val();
          }
        }

  Matrix<double,R1,C2> C(_M,_N);
  C = Map<Matrix<double,R1,C2> >(_C,_M,_N);
  
  C = Map<Matrix<double,R1,C1> >(_A,_M,_M)
    .template triangularView<TriView>().solve(C);

  pos = 0;
        for (size_type j = 0; j < _N; j++) {
          for (size_type i = 0; i < _M; i++) {
      _C[pos] = C(i,j);
            _variRefC[pos] = new vari(_C[pos],false);
      pos++;
          }
        }
      }
      
      virtual void chain() {
  using Eigen::Matrix;
        using Eigen::Map;
        Matrix<double,R2,C2> adjB(_M,_N);
        Matrix<double,R1,C2> adjC(_M,_N);

  size_t pos = 0;
        for (size_type j = 0; j < adjC.cols(); j++)
          for (size_type i = 0; i < adjC.rows(); i++)
            adjC(i,j) = _variRefC[pos++]->adj_;

  adjB = Map<Matrix<double,R1,C1> >(_A,_M,_M)
    .template triangularView<TriView>().transpose().solve(adjC);
  
  pos = 0;
        for (size_type j = 0; j < adjB.cols(); j++)
          for (size_type i = 0; i < adjB.rows(); i++)
            _variRefB[pos++]->adj_ += adjB(i,j);
      }
    };
    
    template <int TriView,int R1,int C1,int R2,int C2>
    class mdivide_left_tri_vd_vari : public vari {
    public:
      int _M; // A.rows() = A.cols() = B.rows()
      int _N; // B.cols()
      double* _A;
      double* _C;
      vari** _variRefA;
      vari** _variRefC;

      mdivide_left_tri_vd_vari(const Eigen::Matrix<var,R1,C1> &A,
                               const Eigen::Matrix<double,R2,C2> &B)
      : vari(0.0),
  _M(A.rows()),
  _N(B.cols()),
  _A((double*)stan::agrad::memalloc_.alloc(sizeof(double) 
                                                 * A.rows() * A.cols())),
  _C((double*)stan::agrad::memalloc_.alloc(sizeof(double) 
                                                 * B.rows() * B.cols())),
        _variRefA((vari**)stan::agrad::memalloc_.alloc(sizeof(vari*) 
                   * A.rows() 
                   * (A.rows() + 1) / 2)),
        _variRefC((vari**)stan::agrad::memalloc_.alloc(sizeof(vari*) 
                                                      * B.rows() * B.cols()))
      {
  using Eigen::Matrix;
        using Eigen::Map;

  size_t pos = 0;
  if (TriView == Eigen::Lower) {
    for (size_type j = 0; j < _M; j++)
      for (size_type i = j; i < _M; i++)
        _variRefA[pos++] = A(i,j).vi_;
  } else if (TriView == Eigen::Upper) {
    for (size_type j = 0; j < _M; j++)
      for (size_type i = 0; i < j+1; i++)
        _variRefA[pos++] = A(i,j).vi_;
  } 

  pos = 0;
  for (size_type j = 0; j < _M; j++) {
          for (size_type i = 0; i < _M; i++) {
      _A[pos++] = A(i,j).val();
          }
        }

  Matrix<double,R1,C2> C(_M,_N);
  C = Map<Matrix<double,R1,C1> >(_A,_M,_M)
    .template triangularView<TriView>().solve(B);

  pos = 0;
        for (size_type j = 0; j < _N; j++) {
          for (size_type i = 0; i < _M; i++) {
      _C[pos] = C(i,j);
            _variRefC[pos] = new vari(_C[pos],false);
      pos++;
          }
        }
      }
      
      virtual void chain() {
  using Eigen::Matrix;
        using Eigen::Map;
  Matrix<double,R1,C1> adjA(_M,_M);
        Matrix<double,R1,C2> adjC(_M,_N);
        
  size_t pos = 0;
        for (size_type j = 0; j < adjC.cols(); j++)
          for (size_type i = 0; i < adjC.rows(); i++)
            adjC(i,j) = _variRefC[pos++]->adj_;

  adjA.noalias() = -Map<Matrix<double,R1,C1> >(_A,_M,_M)
    .template triangularView<TriView>()
    .transpose().solve(adjC*Map<Matrix<double,R1,C2> >(_C,_M,_N).transpose());
  
  pos = 0;
  if (TriView == Eigen::Lower) {
    for (size_type j = 0; j < adjA.cols(); j++)
      for (size_type i = j; i < adjA.rows(); i++)
        _variRefA[pos++]->adj_ += adjA(i,j);
  } else if (TriView == Eigen::Upper) {
    for (size_type j = 0; j < adjA.cols(); j++)
      for (size_type i = 0; i < j+1; i++)
        _variRefA[pos++]->adj_ += adjA(i,j);
  }
      }
    };
    
    template <int TriView,int R1,int C1,int R2,int C2>
    inline 
    Eigen::Matrix<var,R1,C2>
    mdivide_left_tri(const Eigen::Matrix<var,R1,C1> &A,
                     const Eigen::Matrix<var,R2,C2> &b) {
      Eigen::Matrix<var,R1,C2> res(b.rows(),b.cols());
      
      stan::math::validate_square(A,"mdivide_left_tri");
      stan::math::validate_multiplicable(A,b,"mdivide_left_tri");
      
      // NOTE: this is not a memory leak, this vari is used in the 
      // expression graph to evaluate the adjoint, but is not needed
      // for the returned matrix.  Memory will be cleaned up with the arena allocator.
      mdivide_left_tri_vv_vari<TriView,R1,C1,R2,C2> *baseVari = new mdivide_left_tri_vv_vari<TriView,R1,C1,R2,C2>(A,b);

      size_t pos = 0;
      for (size_type j = 0; j < res.cols(); j++)
  for (size_type i = 0; i < res.rows(); i++)
    res(i,j).vi_ = baseVari->_variRefC[pos++];
      
      return res;
    }
    template <int TriView,int R1,int C1,int R2,int C2>
    inline 
    Eigen::Matrix<var,R1,C2>
    mdivide_left_tri(const Eigen::Matrix<double,R1,C1> &A,
                     const Eigen::Matrix<var,R2,C2> &b) {
      Eigen::Matrix<var,R1,C2> res(b.rows(),b.cols());
      
      stan::math::validate_square(A,"mdivide_left_tri");
      stan::math::validate_multiplicable(A,b,"mdivide_left_tri");
      
      // NOTE: this is not a memory leak, this vari is used in the 
      // expression graph to evaluate the adjoint, but is not needed
      // for the returned matrix.  Memory will be cleaned up with the arena allocator.
      mdivide_left_tri_dv_vari<TriView,R1,C1,R2,C2> *baseVari = new mdivide_left_tri_dv_vari<TriView,R1,C1,R2,C2>(A,b);

      size_t pos = 0;
      for (size_type j = 0; j < res.cols(); j++)
  for (size_type i = 0; i < res.rows(); i++)
          res(i,j).vi_ = baseVari->_variRefC[pos++];
      
      return res;
    }
    template <int TriView,int R1,int C1,int R2,int C2>
    inline 
    Eigen::Matrix<var,R1,C2>
    mdivide_left_tri(const Eigen::Matrix<var,R1,C1> &A,
                     const Eigen::Matrix<double,R2,C2> &b) {
      Eigen::Matrix<var,R1,C2> res(b.rows(),b.cols());
      
      stan::math::validate_square(A,"mdivide_left_tri");
      stan::math::validate_multiplicable(A,b,"mdivide_left_tri");
      
      // NOTE: this is not a memory leak, this vari is used in the 
      // expression graph to evaluate the adjoint, but is not needed
      // for the returned matrix.  Memory will be cleaned up with the arena allocator.
      mdivide_left_tri_vd_vari<TriView,R1,C1,R2,C2> *baseVari = new mdivide_left_tri_vd_vari<TriView,R1,C1,R2,C2>(A,b);
      
      size_t pos = 0;
      for (size_type j = 0; j < res.cols(); j++)
  for (size_type i = 0; i < res.rows(); i++)
          res(i,j).vi_ = baseVari->_variRefC[pos++];
      
      return res;
    }
    
    template<int R,int C>
    class determinant_vari : public vari {
      int _rows;
      int _cols;
      double* _A;
      vari** _adjARef;
    public:
      determinant_vari(const Eigen::Matrix<var,R,C> &A)
      : vari(determinant_vari_calc(A)), 
        _rows(A.rows()),
        _cols(A.cols()),
        _A((double*)stan::agrad::memalloc_.alloc(sizeof(double) 
                                                 * A.rows() * A.cols())),
        _adjARef((vari**)stan::agrad::memalloc_.alloc(sizeof(vari*) 
                                                      * A.rows() * A.cols()))
      {
        size_t pos = 0;
        for (size_type j = 0; j < _cols; j++) {
          for (size_type i = 0; i < _rows; i++) {
            _A[pos] = A(i,j).val();
            _adjARef[pos++] = A(i,j).vi_;
          }
        }
      }
      static 
      double determinant_vari_calc(const Eigen::Matrix<var,R,C> &A) {
        Eigen::Matrix<double,R,C> Ad(A.rows(),A.cols());
        for (size_type j = 0; j < A.rows(); j++)
          for (size_type i = 0; i < A.cols(); i++)
            Ad(i,j) = A(i,j).val();
        return Ad.determinant();
      }
      virtual void chain() {
        using Eigen::Matrix;
        using Eigen::Map;
        Matrix<double,R,C> adjA(_rows,_cols);
        adjA = (adj_ * val_) * 
          Map<Matrix<double,R,C> >(_A,_rows,_cols).inverse().transpose();
        size_t pos = 0;
        for (size_type j = 0; j < _cols; j++) {
          for (size_type i = 0; i < _rows; i++) {
            _adjARef[pos++]->adj_ += adjA(i,j);
          }
        }
      }
    };
    
    template <int R, int C>
    inline var determinant(const Eigen::Matrix<var,R,C>& m) {
      stan::math::validate_square(m,"determinant");
      return var(new determinant_vari<R,C>(m));
    }

    template<int R,int C>
    class log_determinant_vari : public vari {
      int _rows;
      int _cols;
      double* _A;
      vari** _adjARef;
    public:
      log_determinant_vari(const Eigen::Matrix<var,R,C> &A)
      : vari(log_determinant_vari_calc(A)), 
        _rows(A.rows()),
        _cols(A.cols()),
        _A((double*)stan::agrad::memalloc_.alloc(sizeof(double) 
                                                 * A.rows() * A.cols())),
        _adjARef((vari**)stan::agrad::memalloc_.alloc(sizeof(vari*) 
                                                      * A.rows() * A.cols()))
      {
        size_t pos = 0;
        for (size_type j = 0; j < _cols; j++) {
          for (size_type i = 0; i < _rows; i++) {
            _A[pos] = A(i,j).val();
            _adjARef[pos++] = A(i,j).vi_;
          }
        }
      }
      static 
      double log_determinant_vari_calc(const Eigen::Matrix<var,R,C> &A)
      {
        Eigen::Matrix<double,R,C> Ad(A.rows(),A.cols());
        for (size_type j = 0; j < A.cols(); j++)
          for (size_type i = 0; i < A.rows(); i++)
            Ad(i,j) = A(i,j).val();
        return Ad.fullPivHouseholderQr().logAbsDeterminant();
      }
      virtual void chain() {
        using Eigen::Matrix;
        using Eigen::Map;
        Matrix<double,R,C> adjA(_rows,_cols);
        adjA = adj_ 
          * Map<Matrix<double,R,C> >(_A,_rows,_cols)
          .inverse().transpose();
        size_t pos = 0;
        for (size_type j = 0; j < _cols; j++) {
          for (size_type i = 0; i < _rows; i++) {
            _adjARef[pos++]->adj_ += adjA(i,j);
          }
        }
      }
    };
    
    template <int R, int C>
    inline var log_determinant(const Eigen::Matrix<var,R,C>& m) {
      stan::math::validate_square(m,"log_determinant");
      return var(new log_determinant_vari<R,C>(m));
    }

    /**
     * Return the division of the first scalar by
     * the second scalar.
     * @param[in] v Specified vector.
     * @param[in] c Specified scalar.
     * @return Vector divided by the scalar.
     */
    inline double
    divide(double x, double y) { 
      return x / y; 
    }
    template <typename T1, typename T2>
    inline var
    divide(const T1& v, const T2& c) {
      return to_var(v) / to_var(c);
    }
    /**
     * Return the division of the specified column vector by
     * the specified scalar.
     * @param[in] v Specified vector.
     * @param[in] c Specified scalar.
     * @return Vector divided by the scalar.
     */
    template <typename T1, typename T2, int R, int C>
    inline Eigen::Matrix<var,R,C>
    divide(const Eigen::Matrix<T1, R,C>& v, const T2& c) {
      return to_var(v) / to_var(c);
    }


    
    /**
     * Return the product of two scalars.
     * @param[in] v First scalar.
     * @param[in] c Specified scalar.
     * @return Product of scalars.
     */
    template <typename T1, typename T2>
    inline
    typename boost::math::tools::promote_args<T1,T2>::type
    multiply(const T1& v, const T2& c) {
      return v * c;
    }

    /**
     * Return the product of scalar and matrix.
     * @param[in] c Specified scalar.
     * @param[in] m Matrix.
     * @return Product of scalar and matrix.
     */
    template<typename T1,typename T2,int R2,int C2>
    inline Eigen::Matrix<var,R2,C2> multiply(const T1& c, 
                                             const Eigen::Matrix<T2, R2, C2>& m) {
      // FIXME:  pull out to eliminate overpromotion of one side
      // move to matrix.hpp w. promotion?
      return to_var(m) * to_var(c);
    }

    /**
     * Return the product of scalar and matrix.
     * @param[in] m Matrix.
     * @param[in] c Specified scalar.
     * @return Product of scalar and matrix.
     */
    template<typename T1,int R1,int C1,typename T2>
    inline Eigen::Matrix<var,R1,C1> multiply(const Eigen::Matrix<T1, R1, C1>& m, 
                                             const T2& c) {
      return to_var(m) * to_var(c);
    }
    
    /**
     * Return the product of the specified matrices.  The number of
     * columns in the first matrix must be the same as the number of rows
     * in the second matrix.
     * @param[in] m1 First matrix.
     * @param[in] m2 Second matrix.
     * @return The product of the first and second matrices.
     * @throw std::domain_error if the number of columns of m1 does not match
     *   the number of rows of m2.
     */
    template<int R1,int C1,int R2,int C2>
    inline Eigen::Matrix<var,R1,C2> multiply(const Eigen::Matrix<var,R1,C1>& m1,
                                             const Eigen::Matrix<var,R2,C2>& m2) {
      stan::math::validate_multiplicable(m1,m2,"multiply");
      Eigen::Matrix<var,R1,C2> result(m1.rows(),m2.cols());
      for (int i = 0; i < m1.rows(); i++) {
        typename Eigen::Matrix<var,R1,C1>::ConstRowXpr crow(m1.row(i));
        for (int j = 0; j < m2.cols(); j++) {
          typename Eigen::Matrix<var,R2,C2>::ConstColXpr ccol(m2.col(j));
          if (j == 0) {
            if (i == 0) {
              result(i,j) = var(new dot_product_vv_vari(crow,ccol));
            }
            else {
              dot_product_vv_vari *v2 = static_cast<dot_product_vv_vari*>(result(0,j).vi_);
              result(i,j) = var(new dot_product_vv_vari(crow,ccol,NULL,v2));
            }
          }
          else { 
            if (i == 0) {
              dot_product_vv_vari *v1 = static_cast<dot_product_vv_vari*>(result(i,0).vi_);
              result(i,j) = var(new dot_product_vv_vari(crow,ccol,v1));
            }
            else /* if (i != 0 && j != 0) */ {
              dot_product_vv_vari *v1 = static_cast<dot_product_vv_vari*>(result(i,0).vi_);
              dot_product_vv_vari *v2 = static_cast<dot_product_vv_vari*>(result(0,j).vi_);
              result(i,j) = var(new dot_product_vv_vari(crow,ccol,v1,v2));
            }
          }
        }
      }
      return result;
    }

    /**
     * Return the product of the specified matrices.  The number of
     * columns in the first matrix must be the same as the number of rows
     * in the second matrix.
     * @param[in] m1 First matrix.
     * @param[in] m2 Second matrix.
     * @return The product of the first and second matrices.
     * @throw std::domain_error if the number of columns of m1 does not match
     *   the number of rows of m2.
     */
    template<int R1,int C1,int R2,int C2>
    inline Eigen::Matrix<var,R1,C2> multiply(const Eigen::Matrix<double,R1,C1>& m1,
                                             const Eigen::Matrix<var,R2,C2>& m2) {
      stan::math::validate_multiplicable(m1,m2,"multiply");
      Eigen::Matrix<var,R1,C2> result(m1.rows(),m2.cols());
      for (int i = 0; i < m1.rows(); i++) {
        typename Eigen::Matrix<double,R1,C1>::ConstRowXpr crow(m1.row(i));
        for (int j = 0; j < m2.cols(); j++) {
          typename Eigen::Matrix<var,R2,C2>::ConstColXpr ccol(m2.col(j));
//          result(i,j) = dot_product(crow,ccol);
          if (j == 0) {
            if (i == 0) {
              result(i,j) = var(new dot_product_vd_vari(ccol,crow));
            }
            else {
              dot_product_vd_vari *v2 = static_cast<dot_product_vd_vari*>(result(0,j).vi_);
              result(i,j) = var(new dot_product_vd_vari(ccol,crow,v2,NULL));
            }
          }
          else { 
            if (i == 0) {
              dot_product_vd_vari *v1 = static_cast<dot_product_vd_vari*>(result(i,0).vi_);
              result(i,j) = var(new dot_product_vd_vari(ccol,crow,NULL,v1));
            }
            else /* if (i != 0 && j != 0) */ {
              dot_product_vd_vari *v1 = static_cast<dot_product_vd_vari*>(result(i,0).vi_);
              dot_product_vd_vari *v2 = static_cast<dot_product_vd_vari*>(result(0,j).vi_);
              result(i,j) = var(new dot_product_vd_vari(ccol,crow,v2,v1));
            }
          }
        }
      }
      return result;
    }
    
    /**
     * Return the product of the specified matrices.  The number of
     * columns in the first matrix must be the same as the number of rows
     * in the second matrix.
     * @param[in] m1 First matrix.
     * @param[in] m2 Second matrix.
     * @return The product of the first and second matrices.
     * @throw std::domain_error if the number of columns of m1 does not match
     *   the number of rows of m2.
     */
    template<int R1,int C1,int R2,int C2>
    inline Eigen::Matrix<var,R1,C2> multiply(const Eigen::Matrix<var,R1,C1>& m1,
                                             const Eigen::Matrix<double,R2,C2>& m2) {
      stan::math::validate_multiplicable(m1,m2,"multiply");
      Eigen::Matrix<var,R1,C2> result(m1.rows(),m2.cols());
      for (int i = 0; i < m1.rows(); i++) {
        typename Eigen::Matrix<var,R1,C1>::ConstRowXpr crow(m1.row(i));
        for (int j = 0; j < m2.cols(); j++) {
          typename Eigen::Matrix<double,R2,C2>::ConstColXpr ccol(m2.col(j));
//          result(i,j) = dot_product(crow,ccol);
          if (j == 0) {
            if (i == 0) {
              result(i,j) = var(new dot_product_vd_vari(crow,ccol));
            }
            else {
              dot_product_vd_vari *v2 = static_cast<dot_product_vd_vari*>(result(0,j).vi_);
              result(i,j) = var(new dot_product_vd_vari(crow,ccol,NULL,v2));
            }
          }
          else { 
            if (i == 0) {
              dot_product_vd_vari *v1 = static_cast<dot_product_vd_vari*>(result(i,0).vi_);
              result(i,j) = var(new dot_product_vd_vari(crow,ccol,v1,NULL));
            }
            else /* if (i != 0 && j != 0) */ {
              dot_product_vd_vari *v1 = static_cast<dot_product_vd_vari*>(result(i,0).vi_);
              dot_product_vd_vari *v2 = static_cast<dot_product_vd_vari*>(result(0,j).vi_);
              result(i,j) = var(new dot_product_vd_vari(crow,ccol,v1,v2));
            }
          }
        }
      }
      return result;
    }

    /**
     * Return the scalar product of the specified row vector and
     * specified column vector.  The return is the same as the dot
     * product.  The two vectors must be the same size.
     * @param[in] rv Row vector.
     * @param[in] v Column vector.
     * @return Scalar result of multiplying row vector by column vector.
     * @throw std::domain_error if rv and v are not the same size
     */
    template <int C1,int R2>
    inline var multiply(const Eigen::Matrix<var, 1, C1>& rv, 
                        const Eigen::Matrix<var, R2, 1>& v) {
      if (rv.size() != v.size())
        throw std::domain_error("row vector and vector must be same length in multiply");
      return dot_product(rv, v);
    }
    /**
     * Return the scalar product of the specified row vector and
     * specified column vector.  The return is the same as the dot
     * product.  The two vectors must be the same size.
     * @param[in] rv Row vector.
     * @param[in] v Column vector.
     * @return Scalar result of multiplying row vector by column vector.
     * @throw std::domain_error if rv and v are not the same size
     */
    template <int C1,int R2>
    inline var multiply(const Eigen::Matrix<double, 1, C1>& rv, 
                        const Eigen::Matrix<var, R2, 1>& v) {
      stan::math::validate_multiplicable(rv,v,"multiply");
      return dot_product(rv, v);
    }
    /**
     * Return the scalar product of the specified row vector and
     * specified column vector.  The return is the same as the dot
     * product.  The two vectors must be the same size.
     * @param[in] rv Row vector.
     * @param[in] v Column vector.
     * @return Scalar result of multiplying row vector by column vector.
     * @throw std::domain_error if rv and v are not the same size
     */
    template <int C1,int R2>
    inline var multiply(const Eigen::Matrix<var, 1, C1>& rv, 
                        const Eigen::Matrix<double, R2, 1>& v) {
      stan::math::validate_multiplicable(rv,v,"multiply");
      return dot_product(rv, v);
    }

    inline matrix_v 
    multiply_lower_tri_self_transpose(const matrix_v& L) {
//      stan::math::validate_square(L,"multiply_lower_tri_self_transpose");
      int K = L.rows();
      int J = L.cols();
      matrix_v LLt(K,K);
      if (K == 0) return LLt;
      // if (K == 1) {
      //   LLt(0,0) = L(0,0) * L(0,0);
      //   return LLt;
      // }
      int Knz;
      if (K >= J)
        Knz = (K-J)*J + (J * (J + 1)) / 2;
      else // if (K < J)
        Knz = (K * (K + 1)) / 2;
      vari** vs = (vari**)memalloc_.alloc( Knz * sizeof(vari*) );
      int pos = 0;
      for (int m = 0; m < K; ++m)
        for (int n = 0; n < ((J < (m+1))?J:(m+1)); ++n) {
          vs[pos++] = L(m,n).vi_;
        }
      for (int m = 0, mpos=0; m < K; ++m, mpos += (J < m)?J:m) {
        LLt(m,m) = var(new dot_self_vari(vs + mpos, (J < (m+1))?J:(m+1)));
        for (int n = 0, npos = 0; n < m; ++n, npos += (J < n)?J:n) {
          LLt(m,n) = LLt(n,m) = var(new dot_product_vv_vari(vs + mpos, vs + npos, (J < (n+1))?J:(n+1)));
        }
      }
      return LLt;
    }

    /**
     * Returns the result of post-multiplying a matrix by its
     * own transpose.
     * @param M Matrix to multiply.
     * @return M times its transpose.
     */
    inline matrix_v
    tcrossprod(const matrix_v& M) {
      if (M.rows() == 0)
        return matrix_v(0,0);
      if (M.rows() == 1)
        return M * M.transpose();

      // WAS JUST THIS
      // matrix_v result(M.rows(),M.rows());
      // return result.setZero().selfadjointView<Eigen::Upper>().rankUpdate(M);

      matrix_v MMt(M.rows(),M.rows());

      vari** vs 
        = (vari**)memalloc_.alloc((M.rows() * M.cols() ) * sizeof(vari*));
      int pos = 0;
      for (int m = 0; m < M.rows(); ++m)
        for (int n = 0; n < M.cols(); ++n)
          vs[pos++] = M(m,n).vi_;
      for (int m = 0; m < M.rows(); ++m)
        MMt(m,m) = var(new dot_self_vari(vs + m * M.cols(),M.cols()));
      for (int m = 0; m < M.rows(); ++m) {
        for (int n = 0; n < m; ++n) {
          MMt(m,n) = var(new dot_product_vv_vari(vs + m * M.cols(),
                                                 vs + n * M.cols(),
                                                 M.cols()));
          MMt(n,m) = MMt(m,n);
        }
      }
      return MMt;
    }

    /**
     * Returns the result of pre-multiplying a matrix by its
     * own transpose.
     * @param M Matrix to multiply.
     * @return Transpose of M times M
     */
    inline matrix_v
    crossprod(const matrix_v& M) {
      return tcrossprod(M.transpose());
    }


    // FIXME:  double val?
    inline void assign_to_var(stan::agrad::var& var, const double& val) {
      var = val;
    }
    inline void assign_to_var(stan::agrad::var& var, const stan::agrad::var& val) {
      var = val;
    }
    // FIXME:  int val?
    inline void assign_to_var(int& n_lhs, const int& n_rhs) {
      n_lhs = n_rhs;  // FIXME: no call -- just filler to instantiate
    }
    // FIXME:  double val?
    inline void assign_to_var(double& n_lhs, const double& n_rhs) {
      n_lhs = n_rhs;  // FIXME: no call -- just filler to instantiate
    }
    
    template <typename LHS, typename RHS>
    inline void assign_to_var(std::vector<LHS>& x, const std::vector<RHS>& y) {
      stan::math::validate_matching_sizes(x,y,"assign_to_var");
      for (size_t i = 0; i < x.size(); ++i)
        assign_to_var(x[i],y[i]);
    }
    template <typename LHS, typename RHS, int R, int C>
    inline void assign_to_var(Eigen::Matrix<LHS,R,C>& x, 
                              const Eigen::Matrix<RHS,R,C>& y) {
      stan::math::validate_matching_sizes(x,y,"assign_to_var");
      for (size_type n = 0; n < x.cols(); ++n)
        for (size_type m = 0; m < x.rows(); ++m)
          assign_to_var(x(m,n),y(m,n));
    }

    template <typename LHS, typename RHS, int R, int C>
    inline void assign_to_var(Eigen::Block<LHS>& x,
                              const Eigen::Matrix<RHS,R,C>& y) {
      stan::math::validate_matching_sizes(x,y,"assign_to_var");
      for (size_type n = 0; n < y.cols(); ++n)
        for (size_type m = 0; m < y.rows(); ++m)
          assign_to_var(x(m,n),y(m,n));
    }
    
    template <typename LHS, typename RHS>
    struct needs_promotion {
      enum { value = ( is_constant_struct<RHS>::value 
                       && !is_constant_struct<LHS>::value) };
    };
    
    template <bool PromoteRHS, typename LHS, typename RHS>
    struct assigner {
      static inline void assign(LHS& /*var*/, const RHS& /*val*/) {
        throw std::domain_error("should not call base class of assigner");
      }
    };
    
    template <typename LHS, typename RHS>
    struct assigner<false,LHS,RHS> {
      static inline void assign(LHS& var, const RHS& val) {
        var = val; // no promotion of RHS
      }
    };

    template <typename LHS, typename RHS>
    struct assigner<true,LHS,RHS> {
      static inline void assign(LHS& var, const RHS& val) {
        assign_to_var(var,val); // promote RHS
      }
    };
    
    
    template <typename LHS, typename RHS>
    inline void assign(Eigen::Block<LHS> var, const RHS& val) {
      assigner<needs_promotion<Eigen::Block<LHS>,RHS>::value, Eigen::Block<LHS>, RHS>::assign(var,val);
    }
    
    template <typename LHS, typename RHS>
    inline void assign(LHS& var, const RHS& val) {
      assigner<needs_promotion<LHS,RHS>::value, LHS, RHS>::assign(var,val);
    }

    void stan_print(std::ostream* o, const var& x) {
      *o << x.val();
    }

  }
}


#endif

