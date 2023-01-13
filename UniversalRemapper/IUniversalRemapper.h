//
// Created by theli on 12/23/2022.
//

#ifndef UNIVERSALREMAPPER_IUNIVERSALREMAPPER_H
#define UNIVERSALREMAPPER_IUNIVERSALREMAPPER_H


class IUniversalRemapper {
public:
    virtual ~IUniversalRemapper() = default;

    virtual void DoNothing() = 0;
};


#endif //UNIVERSALREMAPPER_IUNIVERSALREMAPPER_H
