#include <QCoreApplication>
#include <QDebug>

#include <boost/serialization/access.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/export.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/vector.hpp>

#include <iostream>
#include <memory>
#include <sstream>
#include <fstream>

#include <vector>
#include <algorithm>

using namespace std;

class base {
  public:
    int data1;

    friend class boost::serialization::access;

    template<typename Archive>
    void serialize(Archive & ar, const unsigned int file_version) {
        ar & data1;
    }

  public:
    base() {};
    base(int _d) : data1(_d) {}
    virtual void foo() const {std::cout << "base" << std::endl;}
};

class derived : public base {
  public:
    int data2;

    friend class boost::serialization::access;

    template<typename Archive>
    void serialize(Archive & ar, const unsigned int file_version) {
        ar & boost::serialization::base_object<base>(*this);
        ar & data2;
    }

  public:
    derived() {};
    derived(int _b, int _d) : base(_b), data2(_d) {}
    virtual void foo() const {std::cout << "derived" << std::endl;}
};

BOOST_CLASS_EXPORT(derived);

void f( base* p) {
    p->foo();
}

template<typename Archive>
void serialize(Archive& ar, std::vector<base*>& objs, const unsigned version) {
  ar & objs;
}

int main(int argc, char *argv[]) {

    QCoreApplication a(argc, argv);

    QFile newFile( "new");
    newFile.open(QIODevice::WriteOnly);

        newFile.seek( 0);
        newFile.write( QByteArray());

    // client
    // Assign to base type
//    base* b1 = new derived(1, 2);

//    std::ostringstream oss;
//    boost::archive::text_oarchive oa(oss);
//    oa & b1;

//    // server
//    // Retrieve derived type from base
//    base* b2;

//    std::istringstream iss(oss.str());
//    boost::archive::text_iarchive ia(iss);
//    {
//        base *temp;
//        ia & temp;
//        b2 = temp;
//    }
//    cout << b2->data1 << endl;
//    cout << (dynamic_cast<derived*>(b2))->data2 << endl;
//    b2->foo();
//////////////////////////////////////////////////////////
    //client
    std::ofstream oss2("file_arch.dat");
    boost::archive::text_oarchive oa2(oss2);
    std::vector<base*> v;
    qDebug()<<"s:"<<v.size();
    v.assign( 2, new base(0));
    v.assign( 2, new derived( 1, 2));
    v.assign( 2, new base(0));
    v.assign( 2, new derived( 3, 4));
    qDebug()<<v.size();

    oa2 & v;
    qDebug()<<"-------";
//    //server
    std::ifstream iss2("file_arch",std::ios::binary);
    boost::archive::text_iarchive ia2(iss2);
//    std::vector<base*> temp2, v2;
//    qDebug()<<"t:"<<temp2.size();
//    ia2 & temp2;
//    v2 = temp2;

//    //std::for_each( v2.begin(), v2.end(), f);
//    qDebug()<<temp2.size();

    return a.exec();
}
