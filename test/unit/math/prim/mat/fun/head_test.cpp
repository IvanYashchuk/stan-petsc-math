#include <stan/math/prim/mat.hpp>
#include <gtest/gtest.h>
#include <vector>

TEST(MathMatrixHead, HeadVector1) {
  using stan::math::head;
  Eigen::VectorXd v(3);
  v << 1, 2, 3;
  EXPECT_EQ(0, head(v, 0).size());
}

TEST(MathMatrixHead, HeadVector2) {
  using stan::math::head;
  Eigen::VectorXd v(3);
  v << 1, 2, 3;
  EXPECT_EQ(3, head(v, 3).size());
}
TEST(MathMatrixHead, HeadVector3) {
  using stan::math::head;
  Eigen::VectorXd v(3);
  v << 1, 2, 3;
  EXPECT_THROW(head(v, 4), std::out_of_range);
}
TEST(MathMatrixHead, HeadVector4) {
  using stan::math::head;
  Eigen::VectorXd v(3);
  v << 1, 2, 3;

  Eigen::VectorXd v01 = head(v, 2);
  EXPECT_EQ(2, v01.size());
  for (int n = 0; n < 2; ++n)
    EXPECT_FLOAT_EQ(v[n], v01[n]);
}

TEST(MathMatrixHead, HeadRowVector1) {
  using stan::math::head;
  Eigen::RowVectorXd v(3);
  v << 1, 2, 3;
  EXPECT_EQ(0, head(v, 0).size());
}
TEST(MathMatrixHead, HeadRowVector2) {
  using stan::math::head;
  Eigen::RowVectorXd v(3);
  v << 1, 2, 3;
  EXPECT_EQ(3, head(v, 3).size());
}
TEST(MathMatrixHead, HeadRowVector3) {
  using stan::math::head;
  Eigen::RowVectorXd v(3);
  v << 1, 2, 3;
  EXPECT_THROW(head(v, 4), std::out_of_range);
}
TEST(MathMatrixHead, HeadRowVector4) {
  using stan::math::head;
  Eigen::RowVectorXd v(3);
  v << 1, 2, 3;
  std::vector<size_t> vind{1, 2, 1};
  std::vector<int> ns{2, 2, 2};

  std::vector<Eigen::RowVectorXd> st_v{v, v, v};
  std::vector<Eigen::RowVectorXd> st_t = head(st_v, vind);
  std::vector<Eigen::RowVectorXd> st_t2 = head(st_v, ns);

  Eigen::RowVectorXd v01 = head(v, 2);
  EXPECT_EQ(2, v01.size());
  for (int n = 0; n < 2; ++n)
    EXPECT_FLOAT_EQ(v[n], v01[n]);
}

TEST(MathMatrixHead, HeadStdVector1) {
  using stan::math::head;
  std::vector<int> v;
  v.push_back(1);
  v.push_back(2);
  v.push_back(3);
  EXPECT_EQ(0U, head(v, 0).size());
}
TEST(MathMatrixHead, HeadStdVector2) {
  using stan::math::head;
  std::vector<int> v;
  v.push_back(1);
  v.push_back(2);
  v.push_back(3);
  EXPECT_EQ(3U, head(v, 3).size());
}
TEST(MathMatrixHead, HeadStdVector3) {
  using stan::math::head;
  std::vector<int> v;
  v.push_back(1);
  v.push_back(2);
  v.push_back(3);
  std::vector<int> ns{2, 2, 2};
  std::vector<std::vector<int>> st_v{v, v, v};
  std::vector<std::vector<int>> st_t = head(st_v, 2);
  std::vector<std::vector<int>> st_t2 = head(st_v, ns);
  EXPECT_THROW(head(v, 4), std::out_of_range);
}
TEST(MathMatrixHead, HeadStdVector4) {
  using stan::math::head;
  std::vector<int> v;
  v.push_back(1);
  v.push_back(2);
  v.push_back(3);
  std::vector<int> v01 = head(v, 2);
  EXPECT_EQ(2U, v01.size());
  for (int n = 0; n < 2; ++n)
    EXPECT_FLOAT_EQ(v[n], v01[n]);
}
