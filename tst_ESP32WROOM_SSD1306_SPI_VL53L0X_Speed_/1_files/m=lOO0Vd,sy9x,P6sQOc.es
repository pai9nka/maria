loaded_h_0(function(_){var window=this;
_.v("lOO0Vd");
_.zmb=new _.LOa(_.gSa);
_.x();
var Bmb;Bmb=function(a){if(a.eNc){const b=Date.now()-a.Tge;return a.eNc(a.lHb+1,b)}return Math.random()*Math.min(a.RTd*Math.pow(a.ubc,a.lHb),a.L0d)};_.Cmb=function(a){if(!a.ZVa())throw Error("Ue`"+a.omb);++a.lHb;a.tbc=Bmb(a)};_.Dmb=class{constructor(a,b,c,d,e,f){this.omb=a;this.RTd=b;this.ubc=c;this.L0d=d;this.E$d=e;this.eNc=f||null;this.Tge=Date.now();this.lHb=0;this.tbc=Bmb(this)}rXc(){return this.lHb}ZVa(a){return this.lHb>=this.omb?!1:a!=null?!!this.E$d[a]:!0}};
_.v("P6sQOc");
var Emb=function(a){const b={};_.Ka(a.Ca(),e=>{b[e]=!0});const c=a.Da(),d=a.Ha();return new _.Dmb(a.Ba(),_.ve(c.getSeconds())*1E3,a.Aa(),_.ve(d.getSeconds())*1E3,b)},Fmb=new _.Cq("retryConfigOverrides"),Gmb=function(a,b,c,d){return c.then(e=>e,e=>{if(e instanceof _.gi){if(!e.status||!d.ZVa(e.status.Fr()))throw e;}else if("function"==typeof _.Iib&&e instanceof _.Iib)switch(e.oa){case 103:case 7:case 10:case 101:case 105:case 408:case 425:case 429:case 502:case 503:case 504:break;default:throw e;}if(d&&
!d.ZVa())return _.wh(e);const f=d.tbc;return(new _.Kg(g=>{setTimeout(g,f)})).then(()=>{_.Cmb(d);const g=d.rXc();b=b.vz(_.WWa,g);return Gmb(a,b,a.fetch(b),d)})})};
_.Zf(class{constructor(){this.oa=_.Lf(_.ymb);this.Aa=_.Lf(_.zmb);this.logger=null;const a=_.Lf(_.ohb);this.fetch=a.fetch.bind(a)}x6a(a,b){if(this.Aa.getType(a.Zq())!==1)return new _.thb(a,null,0);var c=this.oa.policy;const d=_.Dq(a,Fmb);var e=null;if(d){e={};if(d.nHb)for(var f of d.nHb)e[f]=!0;else if(c)for(var g of c.Ca())e[g]=!0;let p=1,q=0;f=Infinity;g=2;if(c){p=c.Ba()||p;let u;const A=(u=c.La())==null?void 0:u.getSeconds();q=_.ve(c.Ma().getSeconds())*1E3;f=A!=null?_.ve(A)*1E3:f;g=c.Aa()||g}var h,
k,l;let r;c=(h=d.maxAttempts)!=null?h:p;h=(k=d.Joc)!=null?k:q;k=(l=d.qeb)!=null?l:g;l=(r=d.Ttc)!=null?r:f;e=new _.Dmb(c,h,k,l,e,d.bzd)}else c&&(e=Emb(c));e&&e.ZVa()?(b=Gmb(this,a,b,e),a=new _.thb(a,b,2)):a=new _.thb(a,null,0);return a}},_.Amb);
_.x();
});
// Google Inc.
