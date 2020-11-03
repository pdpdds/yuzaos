#ifndef __LAYER_H__
#define __LAYER_H__

class Level;

class Layer
{
public:

    virtual ~Layer() {}
    
    virtual void Render() = 0;
	virtual void Update(Level* pLevel) = 0;
};


#endif
