#include <iostream>
#include <amp.h>
#include <array>
#include <vector>
#include <utility>


//----------------------------------------------
//traits declaration

template <class M>
struct DataTypeTraits;

template <class M>
struct RankTraits;

template <class M>
struct ArrayTypeTraits;

//----------------------------------------------



// TODO will not work pass 2D -- limitation for now...
// template maybe?
template <int R, typename T>
class Matrix
{
public:
    enum {RANK = R};

    using dataType = T;

    // the choice of using a concurrency::array is to force the data in the 
    using arrayType = concurrency::array<dataType, RANK>;
    using shapeType = concurrency::extent<RANK>;
    using dataContainer = std::vector<dataType>;

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

    arrayType GetData()
    {
        return m_data;
    }
};

template <class Element>
struct Expression
{
    Element m_element;

    Expression(Element & element)
        :m_element(element)
    {
    }

    // operator () avec return Type Traits
    auto operator()() -> ArrayTypeTraits<Element>
    {
        return m_element.GetValues();
    }
};


template <class Element1, class Element2, class Opp>
struct ComplexExpression
{
    using expressionLeft = Expression<Element1>;
    using expressionRight = Expression<Element2>;

    // TODO check if copy is ok
    expressionLeft & m_leftOperand;
    expressionRight & m_rightOperand;

    ComplexExpression(expressionLeft & left, expressionRight & right)
        : m_leftOperand(left)
        , m_rightOperand(right)
    {
    }

    // do something about no reference.... actualy, the return move semantics should do fine...
    auto operator()()-> decltype(Opp::apply(expressionLeft, expressionRight))
    {
        return Opp::apply(m_leftOperand, m_rightOperand)
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

// TODO at some point, constant...
template <class M>
struct DataTypeTraits<Expression<M>>
{
    using dataType = typename DataTypeTraits<M>::dataType;
};

// Complex Expression
template <class Element1, class Element2, class Opp>
struct DataTypeTraits<ComplexExpression<Element1, Element2, Opp>>
{
    using ComplexExpr = ComplexExpression<Element1, Element2, Opp>;

    using LeftDataType = typename DataTypeTraits<typename ComplexExpr::ExpressionLeft>::dataType;
    using RightDataType = typename DataTypeTraits<typename ComplexExpr::ExpressionRight>::dataType;

    using dataType = typename DataTypeTraitsOppBased<LeftDataType, RightDataType, Opp>::dataType;
};


template <class M>
struct RankTraits
{
    enum{ RANK = M::RANK};
};

template <class M>
struct RankTraits<Expression<M>>
{
    enum{ RANK = RankTraits<M>::RANK };
};

template <class Element1, class Element2, class Opp>
struct RankTraits<ComplexExpression<Element1, Element2, Opp>>
{
    using ComplexExpr = ComplexExpression<Element1, Element2, Opp>;

    enum
    {
        LeftDataRank = typename RankTraits<typename ComplexExpr::ExpressionLeft>::RANK,
        RightDataRank = typename RankTraits<typename ComplexExpr::ExpressionRight>::RANK,

        RANK = RankTraitsOppBased<LeftDataRank, RightDataRank, Add>::RANK;
    };
};

// general matrix template
template <class M>
struct ArrayTypeTraits
{
    using arrayType = typename M::arrayType;
};

// TODO at some point, constant...
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

// end traits
//----------------------------------------------



//------------------------------------
// Opps
struct Add
{
    // I still have to find a good return value....a variable maybe??
    template<class LeftExpression, class RightExpression>
    static auto apply(LeftExpression left, RightExpression right)
        ->typename ArrayTypeTraits<ComplexExpression<LeftExpression, RightExpression, Add>>::arrayType
    {
        using ComplexExpr = ComplexExpression<LeftExpression, RightExpression, Add>;
        using arrayType = typename ArrayTypeTraits<ComplexExpr>::arrayType;
        
        const int RANK = RankTraits<ComplexExpr>::RANK;

        //-------------------------------------------------------
        // TODO something for the extent
        arrayType returnValue(concurrency::extent<RANK>(5, 2));
        auto & leftValues = left();
        auto & rightValues = right();

        concurrency::parallel_for_each(
            returnValue.extent,
            [=](concurrency::index<returnValue.extent> idx) restrict(amp)
            {
                returnValue[idx] = leftValues[idx] + rightValues[idx];
            });

        return returnValue;
    }
};
//------------------------------------


//------------------------------------
// opperation based Traits
template <class DataType1, class DataType2>
struct DataTypeTraitsOppBased<DataType1, DataType2, Add>
{
    using dataType = decltype(std::declval<Element1>() + std::declval<DataType2>());
};

template <int Rank1, int Rank2>
struct RankTraitsOppBased<Rank1, Rank2, Add>
{
    enum { RANK = Rank1};
};
//------------------------------------



// TODO operator +
// TODO constructor for Comprex expression that can take an expression, complex expression
//      and the mixture of the 2



typedef Matrix<2, int> Matrix2f;

int main()
{
    using std::vector;
    Matrix2f::shapeType shape(5, 2);

    vector<int> ve1 = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    vector<int> ve2 = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };

    Matrix2f m1(shape);
    Matrix2f m2(shape);


}