#ifndef FIREWORK_H
#define FIREWORK_H

#include <QPoint>
#include <QList>
#include <QColor>
#include <boost/circular_buffer.hpp>

class QPainter;

class Firework
{
    enum State {
        TakeOff,
        BlowUp,
        Hide
    };

    class Spark {
    public:
        QColor color;
        double speed;
        double angle;
        boost::circular_buffer<QPointF> lastCoords;
    };

public:
    explicit Firework();
    void step();
    bool isFinished() const;
    void paint(QPainter *painter);

private:
    State _state;
    Spark _takeoff;
    QList<Spark> _sparks;
    double _blowTime;
    double _hideTime;
    double _time;
    QPoint _lastCoords;
};

#endif // FIREWORK_H
