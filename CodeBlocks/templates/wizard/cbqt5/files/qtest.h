//------------------------------------------------------------------------------
// qtest.h
//------------------------------------------------------------------------------
#ifndef _QTEST_H
#define _QTEST_H
//------------------------------------------------------------------------------
#include <QWidget>
#include <QPushButton>
//------------------------------------------------------------------------------
/** \brief  : Application test for AddOnForQt
 */
class Qtest : public QWidget
{
	Q_OBJECT

    public:
    	/** \brief Test application constructor
    	 *  @param parent : widget parent
    	 *  @param flags : widget flags
    	 */
		Qtest ( QWidget *parent = 0, Qt::WindowFlags flags = 0 );
		/** \brief Test application destructor
    	 */
		virtual ~Qtest ();

    protected:

	protected slots:
		/** \brief  : Message button clicked
		 */
		void press();
		/** \brief : Message button click
		 */
		void message();

    private:
    	/** \brief  : Button
		 */
        QPushButton * button;

    signals:
    	/** \brief  : Button click signal
		 */
        void click();
};

#endif // _QTEST_H
