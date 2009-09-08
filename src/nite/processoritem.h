#pragma once

#define __GEDDEI_BUILD

#include <QtXml>
#include <QtGui>
#include <Geddei>

#include "inputitem.h"
#include "outputitem.h"

static const double cornerSize = 4.0;
static const double statusHeight = 8.0;
static const double statusMargin = 2.0;
static const double portSize = 8.0;
static const double portLateralMargin = 4.0;

class PauseItem;

class ProcessorItem: public QObject, public QGraphicsItem
{
	Q_OBJECT

	friend class InputItem;
	friend class OutputItem;

public:
	ProcessorItem(Processor* _p, Properties const& _pr = Properties(), QString const& _name = QString::null);
	~ProcessorItem()
	{
		delete m_processor;
	}

	Processor* processor() const { return m_processor; }
	QGraphicsItem* statusBar() const { return m_statusBar; }
	QRectF clientArea() const;

	void adjustSize(QSizeF const& _a);

	Properties const& properties() const { return m_properties; }
	void setProperty(QString const& _key, QVariant const& _value);

	bool connectYourself(ProcessorGroup& _group);
	void typesConfirmed();
	void disconnectYourself();

	void tick();

	static void fromDom(QDomElement& _element, QGraphicsScene* _scene);
	void loadYourself(QDomElement& _element);
	void saveYourself(QDomElement& _root, QDomDocument& _doc) const;
	void postLoad()	{}

	virtual QRectF boundingRect() const
	{
		return QRectF(QPointF(-10, -10), m_size + QSize(20.f, 20.f));
	}
	virtual void focusInEvent(QFocusEvent* _e);
	virtual void paint(QPainter* _p, const QStyleOptionGraphicsItem*, QWidget*);
	virtual bool sceneEvent(QEvent* _e) { return QGraphicsItem::sceneEvent(_e); }
	virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* _e);
	virtual void mousePressEvent(QGraphicsSceneMouseEvent* _e);
	virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* _e);
	virtual void hoverMoveEvent(QGraphicsSceneHoverEvent* _e);

	enum { Type = UserType + 1 };
	virtual int type() const { return Type; }

private:
	virtual void timerEvent(QTimerEvent*);

	void propertiesChanged();
	void rejig(Processor* _old = 0, bool _bootStrap = false);

	Processor*			m_processor;
	QGraphicsRectItem*	m_statusBar;
	PauseItem*			m_pauseItem;
	Properties			m_properties;
	QSizeF				m_size;
	int					m_timerId;
	bool				m_resizing;
	QPointF				m_origPosition;
};

