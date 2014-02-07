#include <iostream>
#include <amp.h>
#include <array>
#include <vector>

// TODO will not work pass 2D -- limitation for now...
// template maybe?
template <int D, typename T>
struct Matrix
{
    typedef T dataType;
    typedef concurrency::array<dataType, D> arrayType;
    typedef concurrency::extent<D> shapeType;
    
    Matrix()
    {
        // TODO do something about values
    }

    // I have complete ownership of the data
    void SetData(std::vector<dataType> && data, shapeType shape)
    {
        m_data = arrayType(shape, data.begin(), data.end());
    }

    std::vector<dataType> GetData()
    {
        return m_data;
    }
};


// This is not much usefull for now but can be usefull with more matrix types in the future
template <class M>
struct MatrixTraits;

template<int D, typename T>
struct MatrixTraits<Matrix<D, T>>
{
    enum { Dimension = D};
    typedef T type;
    typedef typename Matrix<D,T>::shapeType shapeType;

    //typedef typename Matrix<D, T>::dataType dataType;
};
// other traits if needed

typedef Matrix<2, int> Matrix2f;

template <class T> // should be a matrix or a vector...either way, we should extract information on it
class Variable
{
public:
    enum { Dimension = MatrixTraits<T>::Dimension };
    typedef typename MatrixTraits<T>::type dataType;
    typedef typename MatrixTraits<T>::shapeType shapeType;
   
private:
    T & m_variable;


public:
    Variable(T & variable)
        :m_variable(variable)
    {
    }
};

//template <typename T>
//struct Constant
//{
//    T m_value;
//
//    Constant(T && value)
//        :m_value(std::move(value))
//    {
//    }
//};

struct Expression
{
    // TODO
};



template <class element1, class element2, class opp>
struct ComplexExpression
{
    // TODO check if copy is ok
    element1 & m_leftOperand;
    element2 & m_rightOperand;

    ComplexExpression(element1 & left, element2 & right)
        : m_leftOperand(left)
        , m_rightOperand(right)
    {
    }

    //auto operator()()->opp::apply(element1, element2)
    auto operator()()->int
    {
        return opp::apply(m_element, m_element2)
    }
};



struct Addition
{
    template<class element1, class element2>
    static void apply(element1, element2)   // TODO return type
    {

    }
};


int main()
{
    using std::vector;
    Matrix2f::shapeType shape(2, 5);

    //vector<int> ve1 = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    //vector<int> ve2 = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };

    Matrix2f m1;
    Matrix2f m2;


}