#pragma once

#include "FactGroup.h"

class VehicleOdometryFactGroup : public FactGroup
{
    Q_OBJECT
    Q_PROPERTY(Fact *x         READ x         CONSTANT)
    Q_PROPERTY(Fact *y         READ y         CONSTANT)
    Q_PROPERTY(Fact *z         READ z         CONSTANT)
    Q_PROPERTY(Fact *xVariance READ xVariance CONSTANT)
    Q_PROPERTY(Fact *yVariance READ yVariance CONSTANT)
    Q_PROPERTY(Fact *zVariance READ zVariance CONSTANT)
    Q_PROPERTY(Fact *quality   READ quality   CONSTANT)

public:
    explicit VehicleOdometryFactGroup(QObject *parent = nullptr);

    Fact *x()         { return &_xFact; }
    Fact *y()         { return &_yFact; }
    Fact *z()         { return &_zFact; }
    Fact *xVariance() { return &_xVarianceFact; }
    Fact *yVariance() { return &_yVarianceFact; }
    Fact *zVariance() { return &_zVarianceFact; }
    Fact *quality()   { return &_qualityFact; }

    // Overrides from FactGroup
    void handleMessage(Vehicle *vehicle, const mavlink_message_t &message) final;

private:
    Fact _xFact         = Fact(0, QStringLiteral("x"),         FactMetaData::valueTypeDouble);
    Fact _yFact         = Fact(0, QStringLiteral("y"),         FactMetaData::valueTypeDouble);
    Fact _zFact         = Fact(0, QStringLiteral("z"),         FactMetaData::valueTypeDouble);
    Fact _xVarianceFact = Fact(0, QStringLiteral("xVariance"), FactMetaData::valueTypeDouble);
    Fact _yVarianceFact = Fact(0, QStringLiteral("yVariance"), FactMetaData::valueTypeDouble);
    Fact _zVarianceFact = Fact(0, QStringLiteral("zVariance"), FactMetaData::valueTypeDouble);
    Fact _qualityFact   = Fact(0, QStringLiteral("quality"),   FactMetaData::valueTypeInt8);
};
