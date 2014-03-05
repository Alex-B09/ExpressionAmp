#include <iostream>
#include <amp.h>
#include <array>
#include <vector>
#include <utility>


//----------------------------------------------
//traits declaration

// traits is maybe not a good name for what thoses do....
// TODO rename thoses -- maybe
template <class M>
struct DataTypeTraits;

template <class M>
struct RankTraits;

template <class M>
struct ArrayTypeTraits;

template<class M>
struct ShapeTraits;

//----------------------------------------------

// TODO will not work pass 2D -- limitation for now...
template <int R, typename T>
class Matrix
{
public:
    enum {RANK = R};
    
    using dataType = T;

    using arrayType = concurrency::array<dataType, RANK>;
    using shapeType = concurrency::extent<RANK>;
    using dataContainer = std::vector<dataType>;

private:
    shapeType m_shape;
    arrayType m_data;

public:
    Matrix(const shapeType & SHAPE)
        :m_shape(SHAPE)
        ,m_data(SHAPE)
    {
    }

    // I have complete ownership of the data
    void SetData(dataContainer && data)
    {
        // TODO continue here.....
        m_data = arrayType(m_shape, data.begin(), data.end());
    }

    arrayType GetData() const
    {
        return m_data;
    }

    shapeType GetShape() const
    {
        return m_shape;
    }

    arrayType operator()() const
    {
        return GetData();
    }
};

template <typename T>
class Constant
{
public:
    using dataType = T;

    // I took the decision to define the same types as in a matrix.
    // This implies that thic class can use the same traits as a
    // matrix
    // This may bite me in the "behind" in later stages...
    // but it works well for now
    using arrayType = concurrency::array<dataType, 1>;
    using shapeType = concurrency::extent<1>;

private:
    dataType m_constantValue;
    shapeType m_shape;
    arrayType m_value;
    
    

public:
    Constant(dataType value)
        :m_constantValue(value)
        ,m_shape(1)
        ,m_value(m_shape, &m_constantValue)
    {
    }

    arrayType GetData() const
    {
        return m_value;
    }

    shapeType GetShape() const
    {
        return shape;
    }

    arrayType operator()() const
    {
        return GetData();
    }
};

template <class Element>
struct Expression
{
    Element & m_element;

    Expression(Element & element)
        :m_element(element)
    {
    }

    auto operator()() const 
        -> decltype(m_element())
    {
        return m_element();
    }
};


template <class Element1, class Element2, class Opp>
struct ComplexExpression
{
    using expressionLeft = Expression<Element1>;
    using expressionRight = Expression<Element2>;

    // TODO check if copy is ok
    expressionLeft m_leftOperand;
    expressionRight m_rightOperand;

    ComplexExpression(expressionLeft left, expressionRight right)
        : m_leftOperand(left)
        , m_rightOperand(right)
    {
    }

    auto operator()() const
        -> decltype(Opp::apply(std::declval<expressionLeft>(), 
                               std::declval<expressionRight>()))
    {
        return Opp::apply(m_leftOperand, m_rightOperand);
    }
};

template <class Element1, class Element2, class Opp>
struct Expression<ComplexExpression<Element1, Element2, Opp>>
{
    using ComplexExpr = ComplexExpression<Element1, Element2, Opp>;
    ComplexExpr m_element;

    Expression(ComplexExpr element)
        :m_element(element)
    {
    }

    // operator () avec return Type Traits
    auto operator()() const
        -> typename ArrayTypeTraits<ComplexExpr>::arrayType
    {
        return m_element();
    }
};

//----------------------------------------------
// traits
template <class Element1, class Element2, class Opp>
struct DataTypeTraitsOppBased;

template <int Rank1, int Rank2, class Opp>
struct RankTraitsOppBased;

// general matrix template
template <class M>
struct DataTypeTraits
{
    using dataType = typename M::dataType;
};


template <class M>
struct DataTypeTraits<Expression<M>>
{
    using dataType = typename DataTypeTraits<M>::dataType;
};

template <class Element1, class Element2, class Opp>
struct DataTypeTraits<ComplexExpression<Element1, Element2, Opp>>
{
    using LeftDataType = typename DataTypeTraits<Element1>::dataType;
    using RightDataType = typename DataTypeTraits<Element2>::dataType;

    using dataType = typename DataTypeTraitsOppBased<LeftDataType, RightDataType, Opp>::dataType;
};

template <class M>
struct RankTraits
{
    enum{ RANK = M::RANK};
};

template <typename T>
struct RankTraits<Constant<T>>
{
    enum{ RANK = 1 };
};

template <class M>
struct RankTraits<Expression<M>>
{
    enum{ RANK = RankTraits<M>::RANK };
};

template <class Element1, class Element2, class Opp>
struct RankTraits<ComplexExpression<Element1, Element2, Opp>>
{
    enum
    {
        LeftDataRank = RankTraits<Element1>::RANK,
        RightDataRank = RankTraits<Element2>::RANK,

        RANK = RankTraitsOppBased<LeftDataRank, RightDataRank, Add>::RANK
    };
};

// general matrix template
template <class M>
struct ArrayTypeTraits
{
    using arrayType = typename M::arrayType;
};

template <class M>
struct ArrayTypeTraits<Expression<M>>
{
    using arrayType = typename ArrayTypeTraits<M>::arrayType;
};

// Complex Expression
template <class Element1, class Element2, class Opp>
struct ArrayTypeTraits<ComplexExpression<Element1, Element2, Opp>>
{
    using ComplexExpr = ComplexExpression<Element1, Element2, Opp>;
    using dataType = typename DataTypeTraits<ComplexExpr>::dataType;

    enum { RANK = RankTraits<ComplexExpr>::RANK };

    using arrayType = concurrency::array<dataType, RANK>;
};

template <typename type, int Rank>
struct ShapeTraits<Matrix<Rank, type>>
{
    using shapeType = typename Matrix<Rank, type>::shapeType;
};

template <typename T>
struct ShapeTraits<Constant<T>>
{
    using shapeType = typename Constant<T>::shapeType;
};

template <class M>
struct ShapeTraits<Expression<M>>
{
    using shapeType = typename ShapeTraits<M>::shapeType;
};

template <class Element1, class Element2, class Opp>
struct ShapeTraits<ComplexExpression<Element1, Element2, Opp>>
{
    // TODO apply the same kind of magic that was applied to DataTypeTraits
    //      to get the right shape applied to the right opp
    //      For now, it will only work with opp that dont change the rank

    using ComplexExpr = ComplexExpression<Element1, Element2, Opp>;

    enum { RANK = RankTraits<ComplexExpr>::RANK };

    using shapeType = concurrency::extent<RANK>;
};

// end traits
//----------------------------------------------

//----------------------------------------------
// helper function


// TODO there is still work to do on the GetShape
//      Must I use the fuction?
template <typename type, int Rank>
auto GetShape(const Matrix<Rank, type> & matrix) 
    -> typename decltype(matrix.GetShape())
{
    return matrix.GetShape();
}

template <typename T>
auto GetShape(const Constant<T> & constant)
    -> decltype(constant.GetShape())
{
    return constant.GetShape();
}

template <class M>
auto GetShape(const Expression<M> & expression) 
    -> decltype(GetShape(expression.m_element))
{
    return GetShape(expression.m_element);
}

template <class Element1, class Element2, class Opp>
auto GetShape(const ComplexExpression<Element1, Element2, Opp> & complexExpression) 
    -> decltype(GetShape(complexExpression.m_leftOperand))
{
    // TODO correct that in the near future to not only take the left element
    return GetShape(complexExpression.m_leftOperand);
}

//----------------------------------------------



//----------------------------------------------
// Opps
struct Add
{
    // I still have to find a good return value....a variable maybe??
    template<class leftExpression, class rightExpression>
    static auto apply(leftExpression left, rightExpression right)
        ->typename ArrayTypeTraits<ComplexExpression<leftExpression, rightExpression, Add>>::arrayType
    {
        using ComplexExpr = ComplexExpression<leftExpression, rightExpression, Add>;
        using arrayType = typename ArrayTypeTraits<ComplexExpr>::arrayType;
        
        const int RANK = RankTraits<ComplexExpr>::RANK;

        // since it's an add, it can either be left or right.
        // TODO once there are constants, there will be work to do here
        arrayType returnValue(GetShape(left));
        auto & leftValues = left();
        auto & rightValues = right();

        concurrency::parallel_for_each(
            returnValue.extent,
            // restrict (amp) is realy picky on how to pass concurrency::array -- only by ref
            [&](concurrency::index<RANK> idx) restrict(amp)
            {
                returnValue[idx] = leftValues[idx] + rightValues[idx];
            });

        return returnValue;
    }


    // the constant will always be on the right....never on the left
    template<class leftExpression, typename T>
    static auto apply(leftExpression left, Expression<Constant<T>> right)
        ->typename ArrayTypeTraits<ComplexExpression<leftExpression, Expression<Constant<T>>, Add>>::arrayType
    {
        using ComplexExpr = ComplexExpression<leftExpression, Expression<Constant<T>>, Add>;
        using arrayType = typename ArrayTypeTraits<ComplexExpr>::arrayType;

        const int RANK = RankTraits<ComplexExpr>::RANK;

        // since the left right expression is a constant, only the left matters
        arrayType returnValue(GetShape(left));
        auto & leftValues = left();
        auto & rightValue = right();

        concurrency::parallel_for_each(
            returnValue.extent,
            // restrict (amp) is realy picky on how to pass concurrency::array -- only by ref
            [&](concurrency::index<RANK> idx) restrict(amp)
        {
            // TODO this is a ugly, ugly hack...must thing of something
            returnValue[idx] = leftValues[idx] + rightValue[0];
        });

        return returnValue;
    }
};
//----------------------------------------------


//----------------------------------------------
// opperation based Traits
template <class DataType1, class DataType2>
struct DataTypeTraitsOppBased<DataType1, DataType2, Add>
{
    using dataType = decltype(std::declval<DataType1>() + std::declval<DataType2>());
};

template <int Rank1, int Rank2>
struct RankTraitsOppBased<Rank1, Rank2, Add>
{
    enum { RANK = Rank1};
};
//----------------------------------------------


template <class Element1, class Element2>
auto operator+(Element1 & left, Element2 & right)
    ->Expression<ComplexExpression<Element1, Element2, Add>>
{
    using ComplexExpr = ComplexExpression<Element1, Element2, Add>;
    return Expression<ComplexExpr>(ComplexExpr(left, right));
}

// constant to the right
template <class Element1, typename T>
auto operator+(Constant<T> & left, Element1 &right)
    ->decltype(right + left)
{
    return right + left;
}

//---------------------------------------
// Test


typedef Matrix<2, int> Matrix2i;
typedef Constant<int> iScalar;

int main()
{
    using std::cout;
    using std::endl;
    
    using std::vector;

    
    iScalar alpha = 3;

    vector<int> v1 = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    vector<int> v2 = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };

    // this is fixed for now
    Matrix2i::shapeType shape(5, 2);
    Matrix2i m1(shape);
    Matrix2i m2(shape);

    // build the expression
    auto t1 = alpha + m1;
    auto t2 = t1 + m2;

    m1.SetData(std::move(v1));
    m2.SetData(std::move(v2));

    // amp array<type> is convertible to vector<type>
    vector<int> v3 = t2();

    for (auto i : v3)
    {
        cout << i << endl;
    }
}
