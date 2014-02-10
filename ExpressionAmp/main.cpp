#include <iostream>
#include <amp.h>
#include <array>
#include <vector>
#include <utility>

// TODO will not work pass 2D -- limitation for now...
// template maybe?
template <int D, typename T>
class Matrix
{
public:
    enum {DIMENSION = D};

    typedef T dataType;

    // the choice of using a concurrency::array is to force the data in the 
    typedef concurrency::array<dataType, D> arrayType;
    typedef concurrency::extent<D> shapeType;
    typedef std::vector<dataType> dataContainer;

private:
    shapeType m_shape;
    dataContainer m_data;

public:
    Matrix(const shapeType & SHAPE)
        :m_shape(SHAPE)
    {
    }

    // I have complete ownership of the data
    void SetData(dataContainer && data)
    {
        // TODO continue here.....
        m_data = arrayType(m_shape, data.begin(), data.end());
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

    int GetValues() const
    {
        // TODO
        return 1;
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

    
    auto operator()() -> decltype(opp::apply(element1, element2))
    {
        return opp::apply(m_element, m_element2)
    }


};

struct Add
{
    // I still have to find a good return value....a variable maybe??
    template<class element1, class element2>
    static auto apply(Variable<element1> left, Variable<element1> right) 
        ->concurrency::array<decltype(std::declval<Variable<element1>::dataType>() + std::declval<Variable<element2>::dataType>()), Variable<element2>::DIMENSION>
    {
        typedef decltype(std::declval<Variable<element1>::dataType>() + std::declval<Variable<element2>::dataType>()), Variable<element2>::DIMENSION> dataType;
        const int DIMENSION = Variable<element2>::DIMENSION;

        
        //-------------------------------------------------------
        // TODO something for the extent
        concurrency::array<dataType, DIMENSION> returnValue(element2.extent);
        auto & leftValues = left.GetValues();
        auto & rightValues = right.GetValues();

        concurrency::parallel_for_each(
            returnValue.extent,
            [=](concurrency::index<DIMENSION> idx) restrict(amp)
            {
                returnValue[idx] = leftValues[idx] + rightValues[idx];
            });


        return concurrency::array<dataType, DIMENSION>();

    }

    // TODO maybe static assert on rank difference...
};

template <class C>
struct OpperationTraits;
// TODO



int main()
{
    using std::vector;
    Matrix2f::shapeType shape(2, 5);

    //vector<int> ve1 = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    //vector<int> ve2 = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };

    Matrix2f m1(shape);
    Matrix2f m2(shape);


}