import dpkt
from scapy.all import *

# def extract_dns_info(pcap_file):
#     dns_table = {}

#     with open(pcap_file, 'rb') as file:
#         pcap = dpkt.pcap.Reader(file)

#         for ts, buf in pcap:
#             eth = dpkt.ethernet.Ethernet(buf)

#             # Check if the packet is an IP packet
#             if isinstance(eth.data, dpkt.ip.IP):
#                 ip = eth.data

#                 # Check if the IP packet is a UDP packet
#                 if isinstance(ip.data, dpkt.udp.UDP):
#                     udp = ip.data

#                     # Check if the UDP packet is a DNS packet
#                     if isinstance(udp.data, dpkt.dns.DNS):
#                         dns = udp.data

#                         # Check if it's a DNS query packet
#                         if dns.qr == dpkt.dns.DNS_Q:
#                             for query in dns.qd:
#                                 domain_name = query.name.decode('utf-8')
#                                 ip_address = ip.src

#                                 dns_table[domain_name] = ip_address

#     return dns_table

# if __name__ == '__main__':
# # Usage example
#     pcap_file = '/home/optimus1010t/college/classes/sem 6/networks_lab/assgn/A1a/a1_fire_final_same_tab.pcap'
#     dns_table = extract_dns_info(pcap_file)

#     # Print the <Domain Name, IP> table
#     for domain_name, ip_address in dns_table.items():
#         print(f'{domain_name}: {ip_address}')


if __name__ == '__main__':

    # Read the .pcap file
    packets = rdpcap('/home/optimus1010t/college/classes/sem 6/networks_lab/assgn/A1a/a1_fire_final_same_tab.pcap')

    # Create a dictionary to store the <Domain Name, IP> pairs
    domain_ip_table = {}

    # Iterate through the packets
    for packet in packets:
        if packet.haslayer(DNSRR):
            # Extract the domain name and IP address from the DNS response
            domain = packet[DNSQR].qname.decode()
            ip = packet[DNSRR].rdata

            # Add the <Domain Name, IP> pair to the table
            domain_ip_table[domain] = ip

    # Print the <Domain Name, IP> table
    for domain, ip in domain_ip_table.items():
        print(f"Domain Name: {domain}, IP: {ip}")
