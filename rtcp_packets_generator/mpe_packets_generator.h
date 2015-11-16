///  @file      mpe_packets_generator.h
///  @brief     RTCP packet generator.
///  @author    peterg at sytel.com
///  @date      03 Nov 2015 3:21 PM
///  @copyright GPL2

#ifndef MPE_PACKETS_GENERATOR_H
#define MPE_PACKETS_GENERATOR_H

#include <QMainWindow>
#include <QSignalMapper>
#include <QUdpSocket>
#include <QValidator>

#include <rtp.h>

namespace Ui {
class Mpe_packets_generator;
}

class QLineEdit;
// SR report blocks + gui info
struct mpe_pg_rb
{
    mpe_rtcp_report_block rb;
    QLineEdit *le[7];
};

class Mpe_packets_generator : public QMainWindow
{
    Q_OBJECT

public:
    explicit Mpe_packets_generator(QWidget *parent = 0);
    ~Mpe_packets_generator();

private:
    int init_socket(const QHostAddress& ip_src, quint16 udp_src);
    void free_socket();
    u_int32_t rtcp_sr_alloc(u_int32_t **sr, u_int8_t sr_n);
    u_int32_t rtcp_sr_build(u_int32_t **pkt_tx);
    u_int32_t rtcp_rr_alloc(u_int32_t **sr, u_int8_t rr_n);
    u_int32_t rtcp_rr_build(u_int32_t **pkt_tx);
    u_int32_t rtcp_bye_len(u_int8_t ssrc_n, u_int8_t reason_len);
    u_int32_t rtcp_bye_alloc(u_int32_t **bye, u_int8_t ssrc_n,
                       u_int8_t reason_len);
    u_int32_t rtcp_bye_build(u_int32_t **bye_tx, u_int32_t ssrc,
                       u_int8_t reason_len, const char* reason);
    void rtcp_build_pkt(QByteArray& data);
    void rtcp_sr_build_report_blocks(int rb_n);
    void rtcp_rr_build_report_blocks(int rb_n);

    Ui::Mpe_packets_generator   *ui;
    QSignalMapper               *mapper, *sr_rb_mapper, *rr_rb_mapper,
                                *bye_mapper;
    QUdpSocket                  *tx_socket;
    QHostAddress                ip_src, ip_dst;
    quint16                     udp_src, udp_dst;
    mpe_rtcp_pt                 rtcp_pt;

    // number of report blocks in SR/RR messages
    int                         rtcp_sr_rb_n, rtcp_rr_rb_n;
    QVector<mpe_pg_rb>          sr_rbs, rr_rbs;

    u_int32_t                   rtcp_bye_ssrc;
    u_int8_t                    rtcp_bye_opt_len;
    QString                     rtcp_bye_opt_reason;
    // validators
    QIntValidator               *le_rtcp_v_v;
    QIntValidator               *le_rtcp_p_v;
    QIntValidator               *le_rtcp_ic_v;
    QIntValidator               *le_rtcp_pt_v;
    QIntValidator               *le_rtcp_len_v;
    QIntValidator               *le_rtcp_bye_opt_len_v;

private slots:
    int send_rtcp(void);
    void rb_rtcp_pt_clicked(int idx);
    void load_default_settings(void);
    // text fields
    void rtcp_hdr_ic_changed(QString text);
    void rtcp_sr_rb_fields_changed(int i);
    void rtcp_rr_rb_fields_changed(int i);
    void rtcp_bye_fields_changed(int i);
};

#endif // MPE_PACKETS_GENERATOR_H
