#ifndef LEDCONTROL_H
#define LEDCONTROL_H

#include <ostream>
#include <mutex>


class LedControl
{
public:
    enum class State {
        On,
        Off
    };

    enum class Color {
        Red,
        Green,
        Blue
    };

    using Rate = unsigned;
    static const unsigned RATE_MAX = 5;

    void setState(State state) {
        std::lock_guard<std::mutex> guard(m_mutex);
        m_state = state;
    }

    State getState() const {
        std::lock_guard<std::mutex> guard(m_mutex);
        return m_state;
    }

    void setColor(Color color) {
        std::lock_guard<std::mutex> guard(m_mutex);
        m_color = color;
    }

    Color getColor() const {
        std::lock_guard<std::mutex> guard(m_mutex);
        return m_color;
    }

    void setRate(Rate rate) {
        std::lock_guard<std::mutex> guard(m_mutex);
        m_rate = rate;
    }

    Rate getRate() const {
        std::lock_guard<std::mutex> guard(m_mutex);
        return m_rate;
    }

    friend std::ostream &operator<<(std::ostream &os, const LedControl &lc) {
        auto color = lc.getColor();
        return os << "state=" << (lc.getState() == State::On ? "on" : "off")
                  << " color=" << (color == Color::Red ? "red" : (color == Color::Green ? "green" : "blue"))
                  << " rate=" << lc.getRate();
    }

private:
    State m_state{ State::Off };
    Color m_color{ Color::Red };
    Rate m_rate{ 0 };
    mutable std::mutex m_mutex;
};

#endif // LEDCONTROLLER_H
