
struct i8237
{
   unsigned start_addr[4], start_counter[4];
   unsigned addr[4], counter[4];
   unsigned char mode[4], mask[4], rq[4];
   unsigned char cmd;

   void out(unsigned char port);
   unsigned char in(unsigned char port);
   void dreq(unsigned chan);
};
