#include <QPainter>
#include <cmath>

#include "firework.h"

const double pi = std::acos(-1);

Firework::Firework()
{
    _blowTime = 2 + qrand() % 4;
    _hideTime = 5 + qrand() % 4;
    _state = TakeOff;
    _takeoff.angle = pi/2 - pi/10 + pi/5 * double(qrand()) / double(RAND_MAX);
    _takeoff.speed = 100.0;
    _time = 0;
}

void Firework::step()
{
    _time += 0.04;

    if (_state == TakeOff) {
        if (_time > _blowTime) {
            _state = BlowUp;
            _time = 0;
            int num = 10 + qrand() % 10;
            for (int i = 0; i < num; i++) {
                Spark s;
                s.lastCoords.set_capacity(30);
                s.angle = 2 * pi * double(qrand()) / double(RAND_MAX);
                s.speed = 30.0;
                s.color = QColor::fromHsv(qrand() % 360, 255, 255);
                _sparks.append(s);
            }
        }
    } else if (_state == BlowUp) {
        if (_time > _hideTime) {
            _state = Hide;
        }
    }
}

bool Firework::isFinished() const
{
    return _state == Hide;
}

void Firework::paint(QPainter *painter)
{
    painter->setPen(QPen(Qt::NoPen));
    if (_state == TakeOff) {
        auto x = _takeoff.speed * std::cos(_takeoff.angle) * _time;
        auto y = _takeoff.speed * std::sin(_takeoff.angle) * _time -
                9.8 * _time * _time / 2;
        _takeoff.lastCoords.push_back(QPointF(x, y));

        QPoint screenCoords(int(x + 500), int(500 - y));
        painter->setBrush(QColor(Qt::red));
        painter->drawEllipse(screenCoords, 3, 3);
        _lastCoords = screenCoords;
        return;
    }

    // blow state
    for (auto &s : _sparks) {
        auto x = s.speed * std::cos(s.angle) * _time;
        auto y = s.speed * std::sin(s.angle) * _time -
                9.8 * _time * _time / 2;

        s.lastCoords.push_back(QPointF(x,y));

        double size = 1;
        double sizeDelta = 6.0 / double(s.lastCoords.size());
        double colorDelta = 255.0 / double(s.lastCoords.size());
        int ch,cs,cv;
        s.color.getHsv(&ch,&cs,&cv);
        QColor color = QColor::fromHsl(ch, cs, int(colorDelta));
        for (auto &p : s.lastCoords) {
            painter->setBrush(color);
            QPoint screenCoords(int(p.x() + _lastCoords.x()), int(_lastCoords.y() - p.y()));
            painter->drawEllipse(screenCoords, int(size), int(size));

            cv = color.value() + int(colorDelta);
            color = QColor::fromHsv(ch, cs, cv > 255? 255 : cv);
            size+=sizeDelta;
        }
    }
}
