#pragma once
#include <QObject>

class CpuStats : public QObject {
    Q_OBJECT
        Q_PROPERTY(int alu READ alu NOTIFY statsChanged)
        Q_PROPERTY(int memory READ memory NOTIFY statsChanged)
        Q_PROPERTY(int control READ control NOTIFY statsChanged)
        Q_PROPERTY(int io READ io NOTIFY statsChanged)

public:
    explicit CpuStats(QObject* parent = nullptr) :
        QObject(parent), _alu(0), _memory(0), _control(0), _io(0)
    {}

    int alu() const { return _alu; }
    int memory() const { return _memory; }
    int control() const { return _control; }
    int io() const { return _io; }

    void updateStats(int alu, int mem, int ctrl, int io) {
        _alu = alu;
        _memory = mem;
        _control = ctrl;
        _io = io;
        emit statsChanged();
    }

signals:
    void statsChanged();

private:
    int _alu, _memory, _control, _io;
};