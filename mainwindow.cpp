#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "Pack.h"

#include <QFileDialog>
#include <QImage>
#include <QPainter>
#include <QResizeEvent>
#include <QWidget>

#include <algorithm>

#include <boost/optional.hpp>

template <typename T> int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}

struct QtImage : public QWidget
{
    int width, height;
    QImage * image, * imageout;
    float progress;

    QtImage(QWidget * parent)
        : QWidget(parent), width(0), height(0), image(0), imageout(0), progress(0.0f)
    {
    }

    ~QtImage()
    {
        delete image;
        delete imageout;
    }

    void incProgress()
    {
        float progress = this->progress + ( 0.2f / (float)width );
        if ( progress >= 1.0f ) progress -= 1.0f;
        this->progress = progress;
        update();
    }

    void mousePressEvent(QMouseEvent * me)
    {
        progress = (float)me->x() / (float)width;
        update();
        me->accept();
    }

    void paintEvent(QPaintEvent *)
    {
        if (!imageout) return;

        {
            int progress_whole = progress * (float)width;

            for (int y = 0; y < height; y++)
            {
                for (int x = 0; x < progress_whole; x++)
                {
                    QColor pixel( image->pixel(x, y) );
                    int h, s, l;
                    pixel.convertTo( QColor::Hsl );
                    pixel.getHsl( &h, &s, &l );
                    pixel.setHsl( h, 0, l );
                    pixel.convertTo( QColor::Rgb );
                    imageout->setPixel(x, y, pixel.rgb() );
                }
                for (int x = progress_whole; x < width; x++)
                {
                    imageout->setPixel(x, y, image->pixel(x, y));
                }
            }

            QPainter painter(imageout);

            QPen pen(QRgb(0));

            QColor color(QRgb(0));

            pen.setWidth(1);

            auto plot_pixel = [&painter, &pen, &color](int x, int y, int alpha) -> void
            {
                int whole_x = x >> 8;
                int whole_y = y >> 8;
                int fract_x = x & 255;
                int fract_y = y & 255;

                color.setAlpha( alpha * (255 - fract_x) * (255 - fract_y) / ( 255 * 255 ) );
                pen.setColor( color );
                painter.setPen( pen );
                painter.drawPoint( whole_x, whole_y );

                if (fract_x)
                {
                    color.setAlpha( alpha * fract_x * (255 - fract_y) / ( 255 * 255 ) );
                    pen.setColor( color );
                    painter.setPen( pen );
                    painter.drawPoint( whole_x + 1, whole_y );
                }

                if (fract_y)
                {
                    color.setAlpha( alpha * (255 - fract_x) * fract_y / ( 255 * 255 ) );
                    pen.setColor( color );
                    painter.setPen( pen );
                    painter.drawPoint( whole_x, whole_y + 1 );
                }

                if (fract_x && fract_y)
                {
                    color.setAlpha( alpha * fract_x * fract_y / ( 255 * 255 ) );
                    pen.setColor( color );
                    painter.setPen( pen );
                    painter.drawPoint( whole_x + 1, whole_y + 1 );
                }
            };

            auto draw_line = [plot_pixel, &pen, &color](int x1, int y1, int x2, int y2, int alpha) -> void
            {
                int dx = x2 - x1;
                int dy = y2 - y1;
                int dxabs = abs(dx);
                int dyabs = abs(dy);
                int sdx = sgn(dx) << 8;
                int sdy = sgn(dy) << 8;
                int px;
                int py;

                plot_pixel( x1, y1, alpha );

                if ( dxabs > dyabs )
                {
                    dyabs = ( dyabs << 8 ) / dxabs;
                    px = ( x1 + sdx + 255 ) & ~255;
                    py = y1 + ( ( dyabs * ( px - x1 ) ) >> 8 );

                    for ( int i = 0; i < dxabs; i += 256 )
                    {
                        plot_pixel( px, py, alpha );
                        px += sdx;
                        py += dyabs;
                    }
                }
                else if ( dyabs > dxabs )
                {
                    dxabs = ( dxabs << 8 ) / dyabs;
                    py = ( y1 + sdy + 255 ) & ~255;
                    px = x1 + ( ( dxabs * ( py - y1 ) ) >> 8 );

                    for ( int i = 0; i < dyabs; i += 256 )
                    {
                        plot_pixel( px, py, alpha );
                        px += dxabs;
                        py += sdy;
                    }
                }
                else
                {
                    px = ( x1 + sdx + 255 ) & ~255;
                    py = ( y1 + sdy + 255 ) & ~255;

                    for ( int i = 0; i < dxabs; i += 256 )
                    {
                        plot_pixel( px, py, alpha );
                        px += sdx;
                        py += sdy;
                    }
                }

                plot_pixel( x2, y2, alpha );
            };

            int progress_fixed = progress * (float)width * 256.0f;

            draw_line( progress_fixed, 0, progress_fixed, height * 256, 255 );
            draw_line( progress_fixed - 256, 0, progress_fixed - 256, height * 256, 192 );
            draw_line( progress_fixed + 256, 0, progress_fixed + 256, height * 256, 192 );
            draw_line( progress_fixed - 512, 0, progress_fixed - 512, height * 256, 64 );
            draw_line( progress_fixed + 512, 0, progress_fixed + 512, height * 256, 64 );
            draw_line( progress_fixed - 768, 0, progress_fixed - 768, height * 256, 16 );
            draw_line( progress_fixed + 768, 0, progress_fixed + 768, height * 256, 16 );
        }

        {
            QPainter painter(this);

            painter.drawImage(0, 0, *imageout);
        }
    }

    void resizeEvent(QResizeEvent * re)
    {
        QSize size = re->size();
        if ( size.width() != width || size.height() != height )
        {
            width = size.width();
            height = size.height();

            delete image;
            image = new QImage(width, height, QImage::Format_RGB32);

            delete imageout;
            imageout = new QImage(width, height, QImage::Format_RGB32);

            QPainter painter(image);
            painter.eraseRect(0, 0, width, height);
        }
    }
};

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    timer(0),
    last_fid(-1)
{
    ui->setupUi(this);

    canvas = new QtImage(ui->widget);
    canvas->setGeometry(0, 0, ui->widget->width(), ui->widget->height());

    ui->lineEdit->setReadOnly(true);

    connect(ui->actionOpen_wavecache_db,
            SIGNAL(triggered()),
            this,
            SLOT(appLoadDB()));
    connect(ui->actionClose,
            SIGNAL(triggered()),
            this,
            SLOT(appCloseDB()));

    ui->spinBox->setEnabled( false );
    connect(ui->spinBox,
            SIGNAL(valueChanged(int)),
            this,
            SLOT(appRenderWave(int)));

    connect(ui->actionExit,
            SIGNAL(triggered()),
            qApp,
            SLOT(quit()));
}

MainWindow::~MainWindow()
{
    delete timer;
    delete canvas;
    delete ui;
}

boost::shared_ptr<sqlite3_stmt> MainWindow::prepare_statement(std::string const& query)
{
    sqlite3_stmt* p = 0;
    sqlite3_prepare_v2(
        db.get(),
        query.c_str(),
        query.size(), &p, 0);
    return boost::shared_ptr<sqlite3_stmt>(p, &sqlite3_finalize);
}

void MainWindow::appLoadDB()
{
    QString filename = QFileDialog::getOpenFileName(this, "Select your wavecache.db...", QString(), "wavecache.db");
    if (filename.length())
    {
        appCloseDB();

        {
            sqlite3* p = 0;
            sqlite3_open(filename.toLocal8Bit().constData(), &p);
            db.reset(p, &sqlite3_close);
        }

        auto stmt = prepare_statement("SELECT COUNT(*) FROM file");
        if (SQLITE_ROW == sqlite3_step(stmt.get()))
        {
            if (sqlite3_column_type(stmt.get(), 0) != SQLITE_NULL)
            {
                int count = sqlite3_column_int(stmt.get(), 0);
                ui->statusBar->showMessage(QString("%1 rows loaded.").arg(count));

                boost::optional<int> fid_min, fid_max;

                stmt = prepare_statement("SELECT fid FROM file ORDER BY fid LIMIT 1");
                if (SQLITE_ROW == sqlite3_step(stmt.get()) &&
                    sqlite3_column_type(stmt.get(), 0) == SQLITE_INTEGER)
                {
                    fid_min = sqlite3_column_int(stmt.get(), 0);
                }
                stmt = prepare_statement("SELECT fid FROM file ORDER BY fid DESC LIMIT 1");
                if (SQLITE_ROW == sqlite3_step(stmt.get()) &&
                    sqlite3_column_type(stmt.get(), 0) == SQLITE_INTEGER)
                {
                    fid_max = sqlite3_column_int(stmt.get(), 0);
                }

                if (fid_min && fid_max)
                {
                    ui->spinBox->setRange(*fid_min, *fid_max);
                }
                else
                {
                    ui->spinBox->setRange(0, count - 1);
                }
                ui->spinBox->setSingleStep(1);
                ui->spinBox->setValue(fid_min ? *fid_min : 0);
                ui->spinBox->setEnabled(true);

                appRenderWave(fid_min ? *fid_min : 0);

                timer = new QTimer(this);
                connect(timer, SIGNAL(timeout()), this, SLOT(onTimer()));
                timer->start(10);
            }
        }
    }
}

void MainWindow::appCloseDB()
{
    db.reset();
    ui->statusBar->clearMessage();
    ui->spinBox->setValue(0);
    ui->spinBox->setEnabled(false);
    ui->lineEdit->setText("");
    last_fid = -1;
    delete timer; timer = 0;
}

void MainWindow::onTimer()
{
    canvas->incProgress();
}

static unsigned count_bits_set(unsigned bits)
{
    unsigned bits_set = 0;
    while (bits)
    {
        if (bits & 1) bits_set++;
        bits >>= 1;
    }
    return bits_set;
}

static const char stmt_back[] = "SELECT fid, location, subsong FROM file WHERE fid <= ? ORDER BY fid DESC LIMIT 1";
static const char stmt_forward[] = "SELECT fid, location, subsong FROM file WHERE fid >= ? ORDER BY fid LIMIT 1";

void MainWindow::appRenderWave(int index)
{
    auto stmt = prepare_statement(index == last_fid - 1 ? stmt_back : stmt_forward);

    sqlite3_bind_int(stmt.get(), 1, index);

    if (SQLITE_ROW != sqlite3_step(stmt.get())) {
        return;
    }

    if (sqlite3_column_type(stmt.get(), 0) != SQLITE_INTEGER ||
        sqlite3_column_type(stmt.get(), 1) != SQLITE_TEXT ||
        sqlite3_column_type(stmt.get(), 2) != SQLITE_INTEGER)
    {
        return;
    }

    int fid = sqlite3_column_int(stmt.get(), 0);

    last_fid = fid;

    ui->spinBox->setValue(fid);

    ui->lineEdit->setText(QString("%1 (subsong %2)").arg((const char *)sqlite3_column_text(stmt.get(), 1)).arg(sqlite3_column_int(stmt.get(), 2)));

    stmt = prepare_statement("SELECT min, max, rms, channels, compression FROM wave WHERE fid = ?");

    sqlite3_bind_int(stmt.get(), 1, fid);

    if (SQLITE_ROW != sqlite3_step(stmt.get())) {
        return;
    }

    boost::optional<int> compression;
    boost::optional<int> channels;

    if (sqlite3_column_type(stmt.get(), 3) != SQLITE_NULL)
        channels = sqlite3_column_int(stmt.get(), 3);
    if (sqlite3_column_type(stmt.get(), 4) != SQLITE_NULL)
        compression = sqlite3_column_int(stmt.get(), 4);

    if (compression && *compression > 1)
        return;

    unsigned channel_count = channels ? count_bits_set(*channels) : 1;

    std::vector< std::vector< std::vector<float> > > waveforms;

    auto clear_and_set = [&stmt, compression, channel_count, &waveforms](size_t col) -> bool
    {
        float const* data = (float const*)sqlite3_column_blob(stmt.get(), col);
        size_t count = sqlite3_column_bytes(stmt.get(), col);

        std::vector< std::vector<float> > list;

        if (compression)
        {
            typedef std::back_insert_iterator< std::vector<char> > Iterator;
            bool (*unpack_func)(void const*, size_t, Iterator) = 0;
            if (*compression == 0)
                unpack_func = &pack::z_unpack<Iterator>;
            if (*compression == 1)
                unpack_func = &pack::lzma_unpack<Iterator>;

            std::vector<char> dst;
            dst.reserve(2048 * channel_count * sizeof(float));
            if (!unpack_func(data, count, std::back_inserter(dst)))
            {
                return false;
            }

            if (dst.size() != channel_count * 2048 * sizeof(float))
            {
                return false;
            }

            for (unsigned c = 0; c < channel_count; ++c)
            {
                std::vector<float> channel;
                float const * fs = (float*)&dst[2048 * c * sizeof(float)];
                std::copy_n(fs, 2048, std::back_inserter(channel));
                list.push_back(channel);
            }
        }
        else
        {
            for (unsigned c = 0; c < channel_count; ++c)
            {
                std::vector<float> channel;
                float const * fs = (float*)&data[2048 * c];
                std::copy_n(fs, 2048, std::back_inserter(channel));
                list.push_back(channel);
            }
        }
        waveforms.push_back(list);
        return true;
    };

    if (!clear_and_set(0) || !clear_and_set(1) || !clear_and_set(2))
        return;

    {
        QRect rect(ui->widget->rect());
        QPainter p(canvas->image);

        p.eraseRect(rect);

        QPen pen(QRgb(0));

        QColor color(QRgb(0));

        pen.setWidth(1);

        auto plot_pixel = [&p, &pen, &color](int x, int y) -> void
        {
            int whole_x = x >> 8;
            int whole_y = y >> 8;
            int fract_x = x & 255;
            int fract_y = y & 255;

            color.setAlpha( (255 - fract_x) * (255 - fract_y) / 255 );
            pen.setColor( color );
            p.setPen( pen );
            p.drawPoint( whole_x, whole_y );

            if (fract_x)
            {
                color.setAlpha( fract_x * (255 - fract_y) / 255 );
                pen.setColor( color );
                p.setPen( pen );
                p.drawPoint( whole_x + 1, whole_y );
            }

            if (fract_y)
            {
                color.setAlpha( (255 - fract_x) * fract_y / 255 );
                pen.setColor( color );
                p.setPen( pen );
                p.drawPoint( whole_x, whole_y + 1 );
            }

            if (fract_x && fract_y)
            {
                color.setAlpha( fract_x * fract_y / 255 );
                pen.setColor( color );
                p.setPen( pen );
                p.drawPoint( whole_x + 1, whole_y + 1 );
            }
        };

        auto draw_line = [plot_pixel, &pen, &color](int x1, int y1, int x2, int y2) -> void
        {
            int dx = x2 - x1;
            int dy = y2 - y1;
            int dxabs = abs(dx);
            int dyabs = abs(dy);
            int sdx = sgn(dx) << 8;
            int sdy = sgn(dy) << 8;
            int px;
            int py;

            plot_pixel( x1, y1 );

            if ( dxabs > dyabs )
            {
                dyabs = ( dyabs << 8 ) / dxabs;
                px = ( x1 + sdx + 255 ) & ~255;
                py = y1 + ( ( dyabs * ( px - x1 ) ) >> 8 );

                for ( int i = 0; i < dxabs; i += 256 )
                {
                    plot_pixel( px, py );
                    px += sdx;
                    py += dyabs;
                }
            }
            else if ( dyabs > dxabs )
            {
                dxabs = ( dxabs << 8 ) / dyabs;
                py = ( y1 + sdy + 255 ) & ~255;
                px = x1 + ( ( dxabs * ( py - y1 ) ) >> 8 );

                for ( int i = 0; i < dyabs; i += 256 )
                {
                    plot_pixel( px, py );
                    px += dxabs;
                    py += sdy;
                }
            }
            else
            {
                px = ( x1 + sdx + 255 ) & ~255;
                py = ( y1 + sdy + 255 ) & ~255;

                for ( int i = 0; i < dxabs; i += 256 )
                {
                    plot_pixel( px, py );
                    px += sdx;
                    py += sdy;
                }
            }

            plot_pixel( x2, y2 );
        };

        float y_scale = (float)rect.height() * 128.0f / (float)channel_count;
        int x_scale = rect.width() * 256 / 2048;

        auto plot_minmax = [draw_line, &color, &waveforms, x_scale, y_scale](int channel, QRgb c) -> void
        {
            color.setRgb( c );
            float offset = 2.0f * channel;
            for ( int i = 0; i < 2048; i++ )
            {
                int x = x_scale * i;
                int y1 = ( 1.0f + waveforms[ 1 ][ channel ][ i ] + offset ) * y_scale;
                int y2 = ( 1.0f + waveforms[ 0 ][ channel ][ i ] + offset ) * y_scale;
                draw_line( x, y1, x, y2 );
            }
        };

        auto plot_rms = [draw_line, &color, &waveforms, x_scale, y_scale](int channel, QRgb c) -> void
        {
            color.setRgb( c );
            float offset = 2.0f * channel;
            for ( int i = 0; i < 2048; i++ )
            {
                int x = x_scale * i;
                int y1 = ( 1.0f + waveforms[ 2 ][ 0 ][ i ] + offset ) * y_scale;
                int y2 = ( 1.0f - waveforms[ 2 ][ 0 ][ i ] + offset ) * y_scale;
                draw_line( x, y1, x, y2 );
            }
        };

        for ( int i = 0; i < channel_count; i++ )
        {
            plot_minmax( i, 0x0080C0 );
            plot_rms( i, 0x7fbfdf );
        }
    }

    canvas->update();
}