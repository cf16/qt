#include "mpe_packets_generator.h"
#include "ui_mpe_packets_generator.h"

#include <mpe_uint_validator.h>

#include <limits>

Mpe_packets_generator::Mpe_packets_generator(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Mpe_packets_generator),
    mapper(new QSignalMapper(this)),
    sr_rb_mapper(new QSignalMapper(this)),
    rr_rb_mapper(new QSignalMapper(this)),
    bye_mapper(new QSignalMapper(this)),
    rtcp_sr_rb_n(0), rtcp_rr_rb_n(0)
{
    ui->setupUi(this);

    // RTCP
    // radio buttons mapping
    QObject::connect(ui->rb_rtcp_sr, SIGNAL(toggled(bool)),
                     mapper, SLOT(map()));
    mapper->setMapping(ui->rb_rtcp_sr, 0);
    ui->rb_rtcp_sr->setChecked(true);
    ui->tab_SR->setEnabled(true);
    rtcp_pt = RTCP_SR;

    QObject::connect(ui->rb_rtcp_rr, SIGNAL(toggled(bool)),
                     mapper, SLOT(map()));
    mapper->setMapping(ui->rb_rtcp_rr, 1);
    ui->rb_rtcp_rr->setChecked(false);
    ui->tab_RR->setEnabled(false);

    QObject::connect(ui->rb_rtcp_sdes, SIGNAL(toggled(bool)),
                     mapper, SLOT(map()));
    mapper->setMapping(ui->rb_rtcp_sdes, 2);
    ui->rb_rtcp_sdes->setChecked(false);
    ui->tab_SDES->setEnabled(false);

    QObject::connect(ui->rb_rtcp_bye, SIGNAL(toggled(bool)),
                     mapper, SLOT(map()));
    mapper->setMapping(ui->rb_rtcp_bye, 3);
    ui->rb_rtcp_bye->setChecked(false);
    ui->tab_BYE->setEnabled(false);

    QObject::connect(ui->rb_rtcp_app, SIGNAL(toggled(bool)),
                     mapper, SLOT(map()));
    mapper->setMapping(ui->rb_rtcp_app, 4);
    ui->rb_rtcp_app->setChecked(false);
    ui->tab_APP->setEnabled(false);

    QObject::connect(mapper, SIGNAL(mapped(int)),
                     this, SLOT(rb_rtcp_pt_clicked(int)));

    // SR report blocks
    QObject::connect(sr_rb_mapper, SIGNAL(mapped(int)),
                     this, SLOT(rtcp_sr_rb_fields_changed(int)));

    // RR report blocks
    QObject::connect(rr_rb_mapper, SIGNAL(mapped(int)),
                     this, SLOT(rtcp_rr_rb_fields_changed(int)));

    // BYE
    QObject::connect(bye_mapper, SIGNAL(mapped(int)),
                     this, SLOT(rtcp_bye_fields_changed(int)));
    // BYE fields
    QObject::connect(ui->le_rtcp_bye_ssrc, SIGNAL(textChanged(QString)),
                     bye_mapper, SLOT(map()));
    bye_mapper->setMapping(ui->le_rtcp_bye_ssrc, 0);
    QObject::connect(ui->le_rtcp_bye_opt_len, SIGNAL(textChanged(QString)),
                     bye_mapper, SLOT(map()));
    bye_mapper->setMapping(ui->le_rtcp_bye_opt_len, 1);
    QObject::connect(ui->le_rtcp_bye_opt_reason, SIGNAL(textChanged(QString)),
                     bye_mapper, SLOT(map()));
    bye_mapper->setMapping(ui->le_rtcp_bye_opt_reason, 2);

    // validators
    ui->le_udp_src->setValidator(new Mpe_uint_validator(0, 65535, this));
    ui->le_udp_dst->setValidator(new Mpe_uint_validator(0, 65535, this));
    le_rtcp_v_v = new QIntValidator(0, 3, this);
    ui->le_rtcp_v->setValidator(le_rtcp_v_v);
    le_rtcp_p_v = new QIntValidator(0, 1, this);
    ui->le_rtcp_p->setValidator(le_rtcp_p_v);
    le_rtcp_ic_v = new QIntValidator(0, 31, this);
    ui->le_rtcp_ic->setValidator(le_rtcp_ic_v);
    le_rtcp_pt_v = new QIntValidator(0, 255, this);
    ui->le_rtcp_pt->setValidator(le_rtcp_pt_v);
    le_rtcp_len_v = new QIntValidator(0, 65535, this);
    ui->le_rtcp_len->setValidator(le_rtcp_len_v);
    le_rtcp_bye_opt_len_v = new QIntValidator(0, 255, this);
    ui->le_rtcp_bye_opt_len->setValidator(le_rtcp_bye_opt_len_v);

    ui->le_rtcp_sr_reporter_ssrc->setValidator(
                new Mpe_uint_validator(0,
                std::numeric_limits<unsigned int>::max(),
                                                this));
    ui->le_rtcp_sr_sr_ts_ntp_sec->setValidator(
                new Mpe_uint_validator(0,
                std::numeric_limits<unsigned int>::max(),
                                                this));
    ui->le_rtcp_sr_sr_ts_ntp_frac->setValidator(
                new Mpe_uint_validator(0,
                std::numeric_limits<unsigned int>::max(),
                                                this));
    ui->le_rtcp_sr_sr_ts_rtp->setValidator(
                new Mpe_uint_validator(0,
                std::numeric_limits<unsigned int>::max(),
                                                this));
    ui->le_rtcp_sr_sr_pkt_count->setValidator(
                new Mpe_uint_validator(0,
                std::numeric_limits<unsigned int>::max(),
                                                this));
    ui->le_rtcp_sr_sr_octet_count->setValidator(
                new Mpe_uint_validator(0,
                std::numeric_limits<unsigned int>::max(),
                                                this));
    ui->le_rtcp_bye_ssrc->setValidator(
                new Mpe_uint_validator(0,
                std::numeric_limits<unsigned int>::max(),
                                                this));


    // buttons
    QObject::connect(ui->btn_rtcp_tx, SIGNAL(clicked(void)),
                     this, SLOT(send_rtcp(void)));
    QObject::connect(ui->btn_default_settings, SIGNAL(clicked(void)),
                     this, SLOT(load_default_settings(void)));

    // text fields changes
    QObject::connect(ui->le_rtcp_ic, SIGNAL(textChanged(QString)),
                     this, SLOT(rtcp_hdr_ic_changed(QString)));

    // default to SR
    ui->rb_rtcp_sr->toggled(true);
    ui->tab_rtcp_body->setCurrentIndex(0);
}

Mpe_packets_generator::~Mpe_packets_generator()
{
    delete ui;
}

void Mpe_packets_generator::load_default_settings(void)
{
    static int flip;

    if (flip == 0)
    {
    // evaluate gui only, don't fill in fields - they
    // will be evaluated on tx independently anyway
    ui->le_ip_src->setText("10.31.5.84");
    ui->le_ip_dst->setText("10.31.5.85");
    ui->le_udp_src->setText("12371");
    ui->le_udp_dst->setText("12381");

    ui->le_rtcp_v->setText("2");
    ui->le_rtcp_p->setText("0");

    flip = 1;
    } else {
        // restore
        ui->le_ip_src->clear();
        ui->le_ip_dst->clear();
        ui->le_udp_src->clear();
        ui->le_udp_dst->clear();

        ui->le_rtcp_v->clear();

        ui->le_rtcp_bye_ssrc->clear();
        ui->le_rtcp_bye_opt_len->clear();
        ui->le_rtcp_bye_opt_reason->clear();
        flip = 0;
    }
    return;
}

void Mpe_packets_generator::rtcp_sr_build_report_blocks(int rb_n)
{
    QWidget *new_page = NULL;

    if (rb_n > rtcp_sr_rb_n)
    {
        // add tabs
        QSizePolicy s_policy;
        for (; rtcp_sr_rb_n < rb_n; rtcp_sr_rb_n++)
        {
            // allocate rb to be txed
            mpe_pg_rb rb;
            memset(&rb, 0, sizeof rb);

            new_page = new QWidget(ui->tab_rtcp_sr_rblocks);
            QGridLayout *g_layout = new QGridLayout(new_page);

            // 0 item
            QLabel *la_reportee_ssrc = new QLabel(new_page);
            la_reportee_ssrc->setText("Reportee SSRC");
            g_layout->addWidget(la_reportee_ssrc);
            QLineEdit *le_reportee_ssrc = new QLineEdit(new_page);
            le_reportee_ssrc->setValidator(new Mpe_uint_validator(0,
                            std::numeric_limits<unsigned int>::max(),
                                                               this));
            QObject::connect(le_reportee_ssrc, SIGNAL(textChanged(QString)),
                             sr_rb_mapper, SLOT(map()));
            sr_rb_mapper->setMapping(le_reportee_ssrc, 7 * rtcp_sr_rb_n + 0);
            g_layout->addWidget(le_reportee_ssrc);
            rb.le[0] = le_reportee_ssrc;

            QHBoxLayout *hb_layout = new QHBoxLayout();
            QLabel *la_loss_fraction = new QLabel(new_page);
            la_loss_fraction->setText("Loss fraction");
            hb_layout->addWidget(la_loss_fraction);
            QLabel *la_cum_n_pkt_lost = new QLabel(new_page);
            la_cum_n_pkt_lost->setText("Cum n pkt lost");
            hb_layout->addWidget(la_cum_n_pkt_lost);
            g_layout->addLayout(hb_layout,2,0);

            // 1 item
            hb_layout = new QHBoxLayout();
            QLineEdit *le_loss_fraction = new QLineEdit(new_page);
            s_policy.setHorizontalPolicy(QSizePolicy::Expanding);
            s_policy.setVerticalPolicy(QSizePolicy::Fixed);
            s_policy.setHorizontalStretch(1);
            le_loss_fraction->setSizePolicy(s_policy);
            le_loss_fraction->setValidator(
                        new QIntValidator(0, 255, this));
            QObject::connect(le_loss_fraction, SIGNAL(textChanged(QString)),
                             sr_rb_mapper, SLOT(map()));
            sr_rb_mapper->setMapping(le_loss_fraction, 7 * rtcp_sr_rb_n + 1);
            hb_layout->addWidget(le_loss_fraction);
            rb.le[1] = le_loss_fraction;

            // 2 item
            QLineEdit *le_cum_n_pkt_lost = new QLineEdit(new_page);
            s_policy.setHorizontalPolicy(QSizePolicy::Expanding);
            s_policy.setVerticalPolicy(QSizePolicy::Fixed);
            s_policy.setHorizontalStretch(3);
            le_cum_n_pkt_lost->setSizePolicy(s_policy);
            le_cum_n_pkt_lost->setValidator(
                        new QIntValidator(0, (2 << 24) - 1, this));
            QObject::connect(le_cum_n_pkt_lost, SIGNAL(textChanged(QString)),
                             sr_rb_mapper, SLOT(map()));
            sr_rb_mapper->setMapping(le_cum_n_pkt_lost, 7 * rtcp_sr_rb_n + 2);
            hb_layout->addWidget(le_cum_n_pkt_lost);
            rb.le[2] = le_cum_n_pkt_lost;

            g_layout->addLayout(hb_layout,3,0);

            // 3 item
            QLabel *la_extended_high_seq = new QLabel(new_page);
            la_extended_high_seq->setText("Extended highest sequence number");
            g_layout->addWidget(la_extended_high_seq);
            QLineEdit *le_extended_high_seq = new QLineEdit(new_page);
            le_extended_high_seq->setValidator(new Mpe_uint_validator(0,
                            std::numeric_limits<unsigned int>::max(),
                                                               this));
            QObject::connect(le_extended_high_seq, SIGNAL(textChanged(QString)),
                             sr_rb_mapper, SLOT(map()));
            sr_rb_mapper->setMapping(le_extended_high_seq, 7 * rtcp_sr_rb_n + 3);
            g_layout->addWidget(le_extended_high_seq);
            rb.le[3] = le_extended_high_seq;

            // item 4
            QLabel *la_jitter = new QLabel(new_page);
            la_jitter->setText("Interarrival jitter");
            g_layout->addWidget(la_jitter);
            QLineEdit *le_jitter = new QLineEdit(new_page);
            le_jitter->setValidator(new Mpe_uint_validator(0,
                            std::numeric_limits<unsigned int>::max(),
                                                               this));
            QObject::connect(le_jitter, SIGNAL(textChanged(QString)),
                             sr_rb_mapper, SLOT(map()));
            sr_rb_mapper->setMapping(le_jitter, 7 * rtcp_sr_rb_n + 4);
            g_layout->addWidget(le_jitter);
            rb.le[4] = le_jitter;

            // item 5
            QLabel *la_ts_lsr = new QLabel(new_page);
            la_ts_lsr->setText("Timestamp of last SR received (LSR)");
            g_layout->addWidget(la_ts_lsr);
            QLineEdit *le_ts_lsr = new QLineEdit(new_page);
            le_ts_lsr->setValidator(new Mpe_uint_validator(0,
                            std::numeric_limits<unsigned int>::max(),
                                                               this));
            QObject::connect(le_ts_lsr, SIGNAL(textChanged(QString)),
                             sr_rb_mapper, SLOT(map()));
            sr_rb_mapper->setMapping(le_ts_lsr, 7 * rtcp_sr_rb_n + 5);
            g_layout->addWidget(le_ts_lsr);
            rb.le[5] = le_ts_lsr;

            // item 6
            QLabel *la_ts_dlsr = new QLabel(new_page);
            la_ts_dlsr->setText("Delay since last SR received (DLSR)");
            g_layout->addWidget(la_ts_dlsr);
            QLineEdit *le_ts_dlsr = new QLineEdit(new_page);
            le_ts_dlsr->setValidator(new Mpe_uint_validator(0,
                            std::numeric_limits<unsigned int>::max(),
                                                               this));
            QObject::connect(le_ts_dlsr, SIGNAL(textChanged(QString)),
                             sr_rb_mapper, SLOT(map()));
            sr_rb_mapper->setMapping(le_ts_dlsr, 7 * rtcp_sr_rb_n + 6);
            g_layout->addWidget(le_ts_dlsr);
            rb.le[6] = le_ts_dlsr;

            new_page->setLayout(g_layout);
            ui->tab_rtcp_sr_rblocks->addTab(new_page,
                             QString::number(rtcp_sr_rb_n));

            sr_rbs.append(rb); // will copy
        }
    } else if (rb_n < rtcp_sr_rb_n)
    {
        // remove tabs
        for ( ; rtcp_sr_rb_n > rb_n; rtcp_sr_rb_n--)
        {
            ui->tab_rtcp_sr_rblocks->removeTab(rtcp_sr_rb_n - 1);
            sr_rbs.removeLast();
        }
    }
    return;
}

void Mpe_packets_generator::rtcp_rr_build_report_blocks(int rb_n)
{
    QWidget *new_page = NULL;

    if (rb_n > rtcp_rr_rb_n)
    {
        // add tabs
        QSizePolicy s_policy;
        for (; rtcp_rr_rb_n < rb_n; rtcp_rr_rb_n++)
        {
            // allocate rb to be txed
            mpe_pg_rb rb;
            memset(&rb, 0, sizeof rb);

            new_page = new QWidget(ui->tab_rtcp_rr_rblocks);
            QGridLayout *g_layout = new QGridLayout(new_page);

            // item 0
            QLabel *la_reportee_ssrc = new QLabel(new_page);
            la_reportee_ssrc->setText("Reportee SSRC");
            g_layout->addWidget(la_reportee_ssrc);
            QLineEdit *le_reportee_ssrc = new QLineEdit(new_page);
            le_reportee_ssrc->setValidator(new Mpe_uint_validator(0,
                            std::numeric_limits<unsigned int>::max(),
                                                               this));
            QObject::connect(le_reportee_ssrc, SIGNAL(textChanged(QString)),
                             rr_rb_mapper, SLOT(map()));
            rr_rb_mapper->setMapping(le_reportee_ssrc, 7 * rtcp_rr_rb_n + 0);
            g_layout->addWidget(le_reportee_ssrc);
            rb.le[0] = le_reportee_ssrc;

            QHBoxLayout *hb_layout = new QHBoxLayout();
            QLabel *la_loss_fraction = new QLabel(new_page);
            la_loss_fraction->setText("Loss fraction");
            hb_layout->addWidget(la_loss_fraction);
            QLabel *la_cum_n_pkt_lost = new QLabel(new_page);
            la_cum_n_pkt_lost->setText("Cum n pkt lost");
            hb_layout->addWidget(la_cum_n_pkt_lost);
            g_layout->addLayout(hb_layout,2,0);

            // item 1
            hb_layout = new QHBoxLayout();
            QLineEdit *le_loss_fraction = new QLineEdit(new_page);
            s_policy.setHorizontalPolicy(QSizePolicy::Expanding);
            s_policy.setVerticalPolicy(QSizePolicy::Fixed);
            s_policy.setHorizontalStretch(1);
            le_loss_fraction->setSizePolicy(s_policy);
            le_loss_fraction->setValidator(
                        new QIntValidator(0, 255, this));
            QObject::connect(le_loss_fraction, SIGNAL(textChanged(QString)),
                             rr_rb_mapper, SLOT(map()));
            rr_rb_mapper->setMapping(le_loss_fraction, 7 * rtcp_rr_rb_n + 1);
            hb_layout->addWidget(le_loss_fraction);
            rb.le[1] = le_loss_fraction;

            // item 2
            QLineEdit *le_cum_n_pkt_lost = new QLineEdit(new_page);
            s_policy.setHorizontalPolicy(QSizePolicy::Expanding);
            s_policy.setVerticalPolicy(QSizePolicy::Fixed);
            s_policy.setHorizontalStretch(3);
            le_cum_n_pkt_lost->setSizePolicy(s_policy);
            le_cum_n_pkt_lost->setValidator(
                        new QIntValidator(0, (2 << 24) - 1, this));
            QObject::connect(le_cum_n_pkt_lost, SIGNAL(textChanged(QString)),
                             rr_rb_mapper, SLOT(map()));
            rr_rb_mapper->setMapping(le_cum_n_pkt_lost, 7 * rtcp_rr_rb_n + 2);
            hb_layout->addWidget(le_cum_n_pkt_lost);
            rb.le[2] = le_cum_n_pkt_lost;

            g_layout->addLayout(hb_layout,3,0);

            // item 3
            QLabel *la_extended_high_seq = new QLabel(new_page);
            la_extended_high_seq->setText("Extended highest sequence number");
            g_layout->addWidget(la_extended_high_seq);
            QLineEdit *le_extended_high_seq = new QLineEdit(new_page);
            le_extended_high_seq->setValidator(new Mpe_uint_validator(0,
                            std::numeric_limits<unsigned int>::max(),
                                                               this));
            QObject::connect(le_extended_high_seq, SIGNAL(textChanged(QString)),
                             rr_rb_mapper, SLOT(map()));
            rr_rb_mapper->setMapping(le_extended_high_seq, 7 * rtcp_rr_rb_n + 3);
            g_layout->addWidget(le_extended_high_seq);
            rb.le[3] = le_extended_high_seq;

            // item 4
            QLabel *la_jitter = new QLabel(new_page);
            la_jitter->setText("Interarrival jitter");
            g_layout->addWidget(la_jitter);
            QLineEdit *le_jitter = new QLineEdit(new_page);
            le_jitter->setValidator(new Mpe_uint_validator(0,
                            std::numeric_limits<unsigned int>::max(),
                                                               this));
            QObject::connect(le_jitter, SIGNAL(textChanged(QString)),
                             rr_rb_mapper, SLOT(map()));
            rr_rb_mapper->setMapping(le_jitter, 7 * rtcp_rr_rb_n + 4);
            g_layout->addWidget(le_jitter);
            rb.le[4] = le_jitter;

            // item 5
            QLabel *la_ts_lsr = new QLabel(new_page);
            la_ts_lsr->setText("Timestamp of last SR received (LSR)");
            g_layout->addWidget(la_ts_lsr);
            QLineEdit *le_ts_lsr = new QLineEdit(new_page);
            le_ts_lsr->setValidator(new Mpe_uint_validator(0,
                            std::numeric_limits<unsigned int>::max(),
                                                               this));
            QObject::connect(le_ts_lsr, SIGNAL(textChanged(QString)),
                             rr_rb_mapper, SLOT(map()));
            rr_rb_mapper->setMapping(le_ts_lsr, 7 * rtcp_rr_rb_n + 5);
            g_layout->addWidget(le_ts_lsr);
            rb.le[5] = le_ts_lsr;

            // item 6
            QLabel *la_ts_dlsr = new QLabel(new_page);
            la_ts_dlsr->setText("Delay since last SR received (DLSR)");
            g_layout->addWidget(la_ts_dlsr);
            QLineEdit *le_ts_dlsr = new QLineEdit(new_page);
            le_ts_dlsr->setValidator(new Mpe_uint_validator(0,
                            std::numeric_limits<unsigned int>::max(),
                                                               this));
            QObject::connect(le_ts_dlsr, SIGNAL(textChanged(QString)),
                             rr_rb_mapper, SLOT(map()));
            rr_rb_mapper->setMapping(le_ts_dlsr, 7 * rtcp_rr_rb_n + 6);
            g_layout->addWidget(le_ts_dlsr);
            rb.le[6] = le_ts_dlsr;

            new_page->setLayout(g_layout);
            ui->tab_rtcp_rr_rblocks->addTab(new_page,
                             QString::number(rtcp_rr_rb_n));

            rr_rbs.append(rb); // will copy
        }
    } else if (rb_n < rtcp_rr_rb_n)
    {
        // remove tabs
        for ( ; rtcp_rr_rb_n > rb_n; rtcp_rr_rb_n--)
        {
            ui->tab_rtcp_rr_rblocks->removeTab(rtcp_rr_rb_n - 1);
            rr_rbs.removeLast();
        }
    }
    return;
}

void Mpe_packets_generator::rtcp_hdr_ic_changed(QString text)
{
    int     pos = 0;
    u_int16_t length;

    switch (rtcp_pt)
    {
    case RTCP_SR:
        // validate
        if (le_rtcp_ic_v->validate(text, pos) != QValidator::Acceptable)
            return;
        // update Length field
        length = 6 + 6 * text.toUInt();
        ui->le_rtcp_len->setText(QString::number(length));
        // populate report blocks
        rtcp_sr_build_report_blocks(text.toUInt());
        break;
    case RTCP_RR:
        // validate
        if (le_rtcp_ic_v->validate(text, pos) != QValidator::Acceptable)
            return;
        // update Length field
        length = 1 + 6 * text.toUInt();
        ui->le_rtcp_len->setText(QString::number(length));
        // populate report blocks
        rtcp_rr_build_report_blocks(text.toUInt());
        break;
    case RTCP_SDES:
        break;
    case RTCP_BYE:
        // TODO
        // handle multi SSRC BYE packets (at the moment only
        // single item BYE is handled)
        break;
    case RTCP_APP:
        break;
    default:
        break;
    }

    return;
}

// SR report blocks fields changed
void Mpe_packets_generator::rtcp_sr_rb_fields_changed(int i)
{
    int quotient, remainder;
    QString log_msg;
    QLineEdit *le;

    quotient = i / 7;
    remainder = i % 7;
    if (quotient > sr_rbs.size() - 1)
    {
        // log
        log_msg = QString("failed to update report block, "
                          "index out of range");
        ui->pte_log->appendPlainText(log_msg);
        return;
    }
    mpe_pg_rb &pg_rb = sr_rbs[quotient];

    le = pg_rb.le[remainder];
    if(le == NULL)
    {
        // log
        log_msg = QString("failed to update report block, "
                          "bad cast");
        ui->pte_log->appendPlainText(log_msg);
        return;
    }

    switch (remainder)
    {
    // fill in field
    // argument is valid since validation has already been done
    case 0:
        // reportee SSRC
        pg_rb.rb.ssrc_reportee = htobe32(le->text().toUInt());
        break;
    case 1:
        // loss fraction
        pg_rb.rb.lost_fraction = le->text().toInt();
        break;
    case 2:
        // cumulative number of packets lost, signed integer
        pg_rb.rb.lost_cum = le->text().toInt();
        break;
    case 3:
        // extended highest sequence number received
        pg_rb.rb.extended_highest_seq = htobe32(le->text().toUInt());
        break;
    case 4:
        // interarrival jitter
        pg_rb.rb.jitter = htobe32(le->text().toUInt());
        break;
    case 5:
        // timestamp of last SR received (LSR)
        pg_rb.rb.lsr = htobe32(le->text().toUInt());
        break;
    case 6:
        // delay since last sender report received (DLSR)
        pg_rb.rb.dlsr = htobe32(le->text().toUInt());
        break;

    default:
        break;
    }

    return;
}

// SR report blocks fields changed
void Mpe_packets_generator::rtcp_rr_rb_fields_changed(int i)
{
    int quotient, remainder;
    QString log_msg;
    QLineEdit *le;

    quotient = i / 7;
    remainder = i % 7;
    if (quotient > rr_rbs.size() - 1)
    {
        // log
        log_msg = QString("failed to update report block, "
                          "index out of range");
        ui->pte_log->appendPlainText(log_msg);
        return;
    }
    mpe_pg_rb &pg_rb = rr_rbs[quotient];

    le = pg_rb.le[remainder];
    if(le == NULL)
    {
        // log
        log_msg = QString("failed to update report block, "
                          "bad cast");
        ui->pte_log->appendPlainText(log_msg);
        return;
    }

    switch (remainder)
    {
    // fill in field
    // argument is valid since validation has already been done
    case 0:
        // reportee SSRC
        pg_rb.rb.ssrc_reportee = htobe32(le->text().toUInt());
        break;
    case 1:
        // loss fraction
        pg_rb.rb.lost_fraction = le->text().toInt();
        break;
    case 2:
        // cumulative number of packets lost, signed integer
        pg_rb.rb.lost_cum = le->text().toInt();
        break;
    case 3:
        // extended highest sequence number received
        pg_rb.rb.extended_highest_seq = htobe32(le->text().toUInt());
        break;
    case 4:
        // interarrival jitter
        pg_rb.rb.jitter = htobe32(le->text().toUInt());
        break;
    case 5:
        // timestamp of last SR received (LSR)
        pg_rb.rb.lsr = htobe32(le->text().toUInt());
        break;
    case 6:
        // delay since last sender report received (DLSR)
        pg_rb.rb.dlsr = htobe32(le->text().toUInt());
        break;

    default:
        break;
    }

    return;
}

// BYE fields changed
void Mpe_packets_generator::rtcp_bye_fields_changed(int i)
{
    u_int32_t   len;
    u_int8_t    reason_len;
    int         pos = 0;
    QString     text;

    switch (i)
    {
    case 0:
        // SSRC
        break;
    case 1:
        // optional reason len
        // validate
        text = ui->le_rtcp_bye_opt_len->text();
        if (text.isEmpty())
        {
            ui->le_rtcp_len->setText(QString::number(1));
        }
        if (le_rtcp_bye_opt_len_v->validate(
                    text, pos)
                != QValidator::Acceptable)
            return;
        // count the pkt len in octets (32bit alligned)
        reason_len = ui->le_rtcp_bye_opt_len->text().toUInt();
        len = rtcp_bye_len(1, reason_len);
        // len in 32bit words - header
        len = (len >> 2) - 1;
        ui->le_rtcp_len->setText(QString::number(len));
        break;
    case 2:
        // optional reason text
        break;
    default:
        break;
    }

    return;
}

int Mpe_packets_generator::init_socket(const QHostAddress& ip_src,
                                        quint16 udp_src)
{
    tx_socket = new QUdpSocket(this);
    if (tx_socket->bind(ip_src, udp_src) == false)
        return -1;
    return 0;
}

void Mpe_packets_generator::free_socket()
{
    tx_socket->close();
    delete tx_socket;
    tx_socket = NULL;
    return;
}

u_int32_t
Mpe_packets_generator::rtcp_sr_alloc(u_int32_t **sr, u_int8_t sr_n)
{
    u_int32_t len;
    len = 4 + 4 + 5 * 4     // RTCP header + reporter SSRC + SR exclusive
            + sr_n * 4 * 6; // report blocks
    *sr = (u_int32_t*)malloc(len);
    if (*sr == NULL)
        return 0;
    memset(*sr, 0, len);
    return len;
}

u_int32_t
Mpe_packets_generator::rtcp_sr_build(u_int32_t **pkt_tx)
{
    struct
    mpe_rtcphdr_common      rtcph = {0,0,0,0,0};
    u_int32_t               plen;
    mpe_rtcp_report_block   *rb_tx = NULL;

    u_int8_t sr_n = ui->le_rtcp_ic->text().toUInt();
    rtcph.pt = RTCP_SR;
    rtcph.version = 2;
    rtcph.count = sr_n;

    // returns the packet size in octets, 32bits alligned
    // and including header
    plen = rtcp_sr_alloc(pkt_tx, sr_n);
    if (plen == 0)
        return 0;
    // in 32bit words excluding the header
    rtcph.length = htobe16((plen >> 2) - 1);
    memcpy(*pkt_tx, &rtcph, sizeof(rtcph));
    // reporter SSRC
    *(*pkt_tx + 1) = htobe32(
                ui->le_rtcp_sr_reporter_ssrc->text().toUInt());
    // SR
    *(*pkt_tx + 2) = htobe32(
                ui->le_rtcp_sr_sr_ts_ntp_sec->text().toUInt());
    *(*pkt_tx + 3) = htobe32(
                ui->le_rtcp_sr_sr_ts_ntp_frac->text().toUInt());
    *(*pkt_tx + 4) = htobe32(
                ui->le_rtcp_sr_sr_ts_rtp->text().toUInt());
    *(*pkt_tx + 5) = htobe32(
                ui->le_rtcp_sr_sr_pkt_count->text().toUInt());
    *(*pkt_tx + 6) = htobe32(
                ui->le_rtcp_sr_sr_octet_count->text().toUInt());
    // report blocks
    rb_tx = (mpe_rtcp_report_block*)(*pkt_tx + 7); // first block
    for (int i = 0; i < sr_rbs.size(); i++)
    {
        const mpe_rtcp_report_block &rb_gui = sr_rbs.at(i).rb;
        // report blocks fields are stored in big endian
        // all but cumulative number of packets lost
        rb_tx->ssrc_reportee = rb_gui.ssrc_reportee;
        rb_tx->lost_fraction = rb_gui.lost_fraction;

        char *cum_host = (char*)&rb_tx->lost_fraction + 1;
        #if __BYTE_ORDER == __LITTLE_ENDIAN
        *cum_host = (rb_gui.lost_cum >> 16) & 0xff;
        *(cum_host + 1) = (rb_gui.lost_cum >> 8) & 0xff;
        *(cum_host + 2) = rb_gui.lost_cum & 0xff;
        #elif __BYTE_ORDER == __BIG_ENDIAN
        *cum_host = rb_gui.lost_cum & 0xff;
        *(cum_host + 1) = (rb_gui.lost_cum >> 8) & 0xff;
        *(cum_host + 2)= (rb_gui.lost_cum >> 16) & 0xff;
#else
#error	"Please fix <asm/byteorder.h>"
        #endif

        rb_tx->extended_highest_seq = rb_gui.extended_highest_seq;
        rb_tx->jitter = rb_gui.jitter;
        rb_tx->lsr = rb_gui.lsr;
        rb_tx->dlsr = rb_gui.dlsr;
        rb_tx++;
    }
    return plen;
}

u_int32_t
Mpe_packets_generator::rtcp_rr_alloc(u_int32_t **sr, u_int8_t rr_n)
{
    u_int32_t len;
    len = 8     // RTCP header + reporter SSRC
            + rr_n * 4 * 6; // report blocks
    *sr = (u_int32_t*)malloc(len);
    if (*sr == NULL)
        return 0;
    memset(*sr, 0, len);
    return len;
}

u_int32_t
Mpe_packets_generator::rtcp_rr_build(u_int32_t **pkt_tx)
{
    struct
    mpe_rtcphdr_common      rtcph = {0,0,0,0,0};
    u_int32_t               plen;
    mpe_rtcp_report_block   *rb_tx = NULL;

    u_int8_t rr_n = ui->le_rtcp_ic->text().toUInt();
    rtcph.pt = RTCP_RR;
    rtcph.version = 2;
    rtcph.count = rr_n;

    // returns the packet size in octets, 32bits alligned
    // and including header
    plen = rtcp_rr_alloc(pkt_tx, rr_n);
    if (plen == 0)
        return 0;
    // in 32bit words excluding the header
    rtcph.length = htobe16((plen >> 2) - 1);
    memcpy(*pkt_tx, &rtcph, sizeof(rtcph));
    // reporter SSRC
    *(*pkt_tx + 1) = htobe32(
                ui->le_rtcp_rr_reporter_ssrc->text().toUInt());
    // report blocks
    rb_tx = (mpe_rtcp_report_block*)(*pkt_tx + 2); // first block
    for (int i = 0; i < rr_rbs.size(); i++)
    {
        const mpe_rtcp_report_block &rb_gui = rr_rbs.at(i).rb;
        // report blocks fields are stored in big endian
        rb_tx->ssrc_reportee = rb_gui.ssrc_reportee;
        rb_tx->lost_fraction = rb_gui.lost_fraction;

        char *cum_host = (char*)&rb_tx->lost_fraction + 1;
        #if __BYTE_ORDER == __LITTLE_ENDIAN
        *cum_host = (rb_gui.lost_cum >> 16) & 0xff;
        *(cum_host + 1) = (rb_gui.lost_cum >> 8) & 0xff;
        *(cum_host + 2) = rb_gui.lost_cum & 0xff;
        #elif __BYTE_ORDER == __BIG_ENDIAN
        *cum_host = rb_gui.lost_cum & 0xff;
        *(cum_host + 1) = (rb_gui.lost_cum >> 8) & 0xff;
        *(cum_host + 2)= (rb_gui.lost_cum >> 16) & 0xff;
#else
#error	"Please fix <asm/byteorder.h>"
        #endif

        rb_tx->extended_highest_seq = rb_gui.extended_highest_seq;
        rb_tx->jitter = rb_gui.jitter;
        rb_tx->lsr = rb_gui.lsr;
        rb_tx->dlsr = rb_gui.dlsr;
        rb_tx++;
    }
    return plen;
}

u_int32_t
Mpe_packets_generator::rtcp_bye_len(
        u_int8_t ssrc_n, u_int8_t reason_len)
{
    u_int32_t len, add_len;
    // + min 4 bytes for optional length/reason
    // if reason_len>0
    if (reason_len == 0)
        add_len = 0;
    else
    {
        // 32bits allignment
        if (reason_len <= 3)
            add_len = 4;
        else
            add_len = (1 + reason_len) +
            (4 - (1 + reason_len) % 4) % 4;
    }
    len = sizeof(struct mpe_rtcphdr_common) +
            ssrc_n * 4 + add_len;
    return len;
}

u_int32_t
Mpe_packets_generator::rtcp_bye_alloc(u_int32_t **bye,
                            u_int8_t ssrc_n, u_int8_t reason_len)
{
    u_int32_t 		len;
    len = rtcp_bye_len(ssrc_n, reason_len);
    *bye = (u_int32_t*)malloc(len);
    if (*bye == NULL)
        return 0;
    memset(*bye, 0, len);
    return len;
}

u_int32_t
Mpe_packets_generator::rtcp_bye_build(u_int32_t **bye_tx,
                                      u_int32_t ssrc,
                   u_int8_t reason_len, const char* reason)
{
    struct
    mpe_rtcphdr_common 	rtcph = {0,0,0,0,0};
    u_int32_t           plen;
    if ((reason_len > 0 && reason == NULL))
        return -1;
    rtcph.pt = RTCP_BYE;
    rtcph.version = 2;
    rtcph.count = 1;

    // returns the packet size in octets, 32bits alligned
    // and including header
    plen = rtcp_bye_alloc(bye_tx, 1, reason_len);
    if (plen == 0)
        return 0;
    // in 32bit words excluding the header
    rtcph.length = htobe16((plen >> 2) - 1);
    memcpy(*bye_tx, &rtcph, sizeof(rtcph));
    *(*bye_tx + 1) = htobe32(ssrc);
    if (reason_len > 0)
    {
        *((char*)*bye_tx + 8) = reason_len;
        memcpy(((char*)*bye_tx + 9), reason, reason_len);
    }

    return plen;
}

void Mpe_packets_generator::rtcp_build_pkt(QByteArray &data)
{
    QString log_msg;
    u_int32_t *pkt_tx = NULL;
    u_int32_t plen;

    switch (rtcp_pt)
    {
    case RTCP_SR:
        plen = rtcp_sr_build(&pkt_tx);
        if (plen == 0)
        {
            // log failure
            log_msg = QString("failed to build the packet");
            ui->pte_log->appendPlainText(log_msg);
            return;
        }
        data.append((char*)pkt_tx, plen);
        free(pkt_tx);
        pkt_tx = NULL;
        break;
    case RTCP_RR:
        plen = rtcp_rr_build(&pkt_tx);
        if (plen == 0)
        {
            // log failure
            log_msg = QString("failed to build the packet");
            ui->pte_log->appendPlainText(log_msg);
            return;
        }
        data.append((char*)pkt_tx, plen);
        free(pkt_tx);
        pkt_tx = NULL;
        break;
    case RTCP_SDES:
        break;
    case RTCP_BYE:
        rtcp_bye_ssrc = ui->le_rtcp_bye_ssrc->text().toUInt();
        rtcp_bye_opt_len = ui->le_rtcp_bye_opt_len->text().toInt();
        rtcp_bye_opt_reason = ui->le_rtcp_bye_opt_reason->text();
        plen = rtcp_bye_build(&pkt_tx, rtcp_bye_ssrc, rtcp_bye_opt_len,
                              rtcp_bye_opt_reason.toStdString().c_str());
        if (plen == 0)
        {
            // log failure
            log_msg = QString("failed to build the packet");
            ui->pte_log->appendPlainText(log_msg);
            return;
        }
        data.append((char*)pkt_tx, plen);
        free(pkt_tx);
        pkt_tx = NULL;
        break;
    case RTCP_APP:
        break;
    default:
        break;
    }

    return;
}

int Mpe_packets_generator::send_rtcp()
{
    QByteArray  data;
    qint64      bytes_tx;
    QString     log_msg;

    ip_src = QHostAddress(ui->le_ip_src->text());
    ip_dst = QHostAddress(ui->le_ip_dst->text());
    udp_src = ui->le_udp_src->text().toUInt();
    udp_dst = ui->le_udp_dst->text().toUInt();
    if (ip_src.isNull() || ip_dst.isNull())
    {
        // error
        log_msg = QString("Please specify IP addresses");
        ui->pte_log->appendPlainText(log_msg);
        return -1;
    }
    if (udp_src == 0 || udp_dst == 0)
    {
        // error
        log_msg = QString("Please specify UDP ports");
        ui->pte_log->appendPlainText(log_msg);
        return -1;
    }
    if (init_socket(ip_src, udp_src) == -1)
    {
        // log error
        log_msg = QString("ERROR. failed to bind the socket, "
                          "plese try different port/address");
        ui->pte_log->appendPlainText(log_msg);
        return -1;
    }
    // send
    rtcp_build_pkt(data);
    bytes_tx = tx_socket->writeDatagram(data, ip_dst, udp_dst);
    if (bytes_tx < data.size())
    {
        // log error
        log_msg = QString("ERROR. tx [%1] bytes to ip [%2] port [%3], "
                          "tx of [%4] bytes expected").arg(
                    QString::number(bytes_tx)).arg(ip_dst.toString()).arg(
                    QString::number(udp_dst)).arg(QString::number(data.size()));
        ui->pte_log->appendPlainText(log_msg);
        return -1;
    }

    // log success
    log_msg = QString("OK. tx [%1] bytes to ip [%2] port [%3]").arg(
                QString::number(bytes_tx), ip_dst.toString(),
                QString::number(udp_dst));
    ui->pte_log->appendPlainText(log_msg);
    free_socket();
    return 0;
}

void Mpe_packets_generator::rb_rtcp_pt_clicked(int idx)
{
    switch (idx)
    {
    case 0:
        // SR
        if (ui->rb_rtcp_sr->isChecked())
        {
            // checked
            ui->tab_SR->setEnabled(true);
            rtcp_pt = RTCP_SR;
            ui->le_rtcp_pt->setText(QString::number(RTCP_SR));
            // disable header Length field as this is counted
            // based on optional reason presence and length
            ui->la_rtcp_h_len->setText("Length is counted "
                     "based on IC field");
            ui->le_rtcp_len->setText("12");
            ui->le_rtcp_len->setEnabled(false);
            // default to 1 report block
            ui->le_rtcp_ic->setText("1");
        } else {
            // unchecked
            ui->tab_SR->setEnabled(false);
        }
        break;
    case 1:
        // RR
        if (ui->rb_rtcp_rr->isChecked())
        {
            // checked
            ui->tab_RR->setEnabled(true);
            rtcp_pt = RTCP_RR;
            ui->le_rtcp_pt->setText(QString::number(RTCP_RR));
            // disable header Length field as this is counted
            // based on optional reason presence and length
            ui->la_rtcp_h_len->setText("Length is counted "
                     "based on IC field");
            ui->le_rtcp_len->setText("7");
            ui->le_rtcp_len->setEnabled(false);
            // default to 1 report block
            ui->le_rtcp_ic->setText("1");
            ui->le_rtcp_len->setText(QString::number(7));
        } else {
            // unchecked
            ui->tab_RR->setEnabled(false);
        }
        break;
    case 2:
        // SDES
        if (ui->rb_rtcp_sdes->isChecked())
        {
            // checked
            ui->tab_SDES->setEnabled(true);
            rtcp_pt = RTCP_SDES;
            ui->le_rtcp_pt->setText(QString::number(RTCP_SDES));
        } else {
            // unchecked
            ui->tab_SDES->setEnabled(false);
        }
        break;
    case 3:
        // BYE
        if (ui->rb_rtcp_bye->isChecked())
        {
            // checked
            ui->tab_BYE->setEnabled(true);
            rtcp_pt = RTCP_BYE;
            // disable header Length field as this is counted
            // based on optional reason presence and length
            ui->la_rtcp_h_len->setText("Length is counted "
                     "based on reason length");
            ui->le_rtcp_len->setEnabled(false);
            ui->le_rtcp_pt->setText(QString::number(RTCP_BYE));
            ui->le_rtcp_len->setText(QString::number(1));

            ui->le_rtcp_bye_ssrc->setText("7");
            ui->le_rtcp_bye_opt_len->setText("23");
            ui->le_rtcp_bye_opt_reason->setText("client has changed SSRC");
            // default to 1 SSRC block
            ui->le_rtcp_ic->setText("1");
            ui->le_rtcp_ic->setEnabled(false);
        } else {
            // unchecked
            ui->tab_BYE->setEnabled(false);
            ui->la_rtcp_h_len->setText("Length");
            ui->le_rtcp_len->setEnabled(true);
            ui->le_rtcp_ic->setEnabled(true);
        }
        break;
    case 4:
        // APP
        if (ui->rb_rtcp_app->isChecked())
        {
            // checked
            ui->tab_APP->setEnabled(true);
            rtcp_pt = RTCP_APP;
            ui->le_rtcp_pt->setText(QString::number(RTCP_APP));
        } else {
            // unchecked
            ui->tab_APP->setEnabled(false);
        }
        break;
    default:
        break;
    }

    emit ui->le_rtcp_ic->textChanged(ui->le_rtcp_ic->text());
    return;
}
