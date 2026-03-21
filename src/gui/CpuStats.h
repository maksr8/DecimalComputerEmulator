#pragma once
#include <QObject>

class CpuStats : public QObject {
    Q_OBJECT
        Q_PROPERTY(int alu READ alu CONSTANT)
        Q_PROPERTY(int memory READ memory CONSTANT)
        Q_PROPERTY(int control READ control CONSTANT)
        Q_PROPERTY(int io READ io CONSTANT)

public:
    explicit CpuStats(QObject* parent = nullptr) : QObject(parent) {}

    int alu() const { return 42; }
    int memory() const { return 28; }
    int control() const { return 15; }
    int io() const { return 15; }
};