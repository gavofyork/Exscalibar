#pragma once

#include <QtXml>
#include <QtGui>
#include <Geddei>

#include "inputitem.h"
#include "outputitem.h"

static const double cornerSize = 4.0;
static const double portSize = 8.0;
static const double portLateralMargin = 4.0;

class ProcessorItem: public QGraphicsItem
{
	friend class InputItem;
	friend class OutputItem;

public:
	ProcessorItem(Processor* _p, Properties const& _pr = Properties());
	~ProcessorItem()
	{
		delete m_processor;
	}

	bool connectYourself()
	{
		return false;
	}
	void disconnectYourself()
	{
	}
	static void fromDom(QDomElement& _element, QGraphicsScene* _scene);
	void loadYourself(QDomElement& _element);
	void saveYourself(QDomElement& _root, QDomDocument& _doc) const;
	void postLoad()	{}

	virtual QRectF boundingRect() const
	{
		return QRectF(QPointF(-10, -10), m_size + QSize(20.f, 20.f));
	}
	virtual void focusInEvent(QFocusEvent* _e) { update(); QGraphicsItem::focusInEvent(_e); }
	virtual void focusOutEvent(QFocusEvent*) { update(); }

	virtual void paint(QPainter* _p, const QStyleOptionGraphicsItem*, QWidget*);

	enum { Type = UserType + 1 };
	virtual int type() const { return Type; }

private:
	ProcessorItem(Processor* _p, Properties const& _pr, QString const& _name);

	Processor*	m_processor;
	Properties	m_properties;
	QSizeF		m_size;
};

