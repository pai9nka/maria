loaded_h_0(function(_){var window=this;
_.v("lOO0Vd");
_.smb=new _.IOa(_.bSa);
_.x();
var umb;umb=function(a){if(a.kNc){const b=Date.now()-a.Zge;return a.kNc(a.xHb+1,b)}return Math.random()*Math.min(a.WTd*Math.pow(a.Bbc,a.xHb),a.R0d)};_.vmb=function(a){if(!a.pWa())throw Error("Ue`"+a.Amb);++a.xHb;a.Abc=umb(a)};_.wmb=class{constructor(a,b,c,d,e,f){this.Amb=a;this.WTd=b;this.Bbc=c;this.R0d=d;this.I$d=e;this.kNc=f||null;this.Zge=Date.now();this.xHb=0;this.Abc=umb(this)}AXc(){return this.xHb}pWa(a){return this.xHb>=this.Amb?!1:a!=null?!!this.I$d[a]:!0}};
_.v("P6sQOc");
var xmb=function(a){const b={};_.Ka(a.Ca(),e=>{b[e]=!0});const c=a.Da(),d=a.Ha();return new _.wmb(a.Ba(),_.ze(c.getSeconds())*1E3,a.Aa(),_.ze(d.getSeconds())*1E3,b)},ymb=new _.Fq("retryConfigOverrides"),zmb=function(a,b,c,d){return c.then(e=>e,e=>{if(e instanceof _.ji){if(!e.status||!d.pWa(e.status.Mr()))throw e;}else if("function"==typeof _.Dib&&e instanceof _.Dib&&e.oa!==103&&e.oa!==7)throw e;if(d&&!d.pWa())return _.Bh(e);const f=d.Abc;return(new _.Pg(g=>{setTimeout(g,f)})).then(()=>{_.vmb(d);const g=
d.AXc();b=b.xz(_.RWa,g);return zmb(a,b,a.fetch(b),d)})})};
_.ag(class{constructor(){this.oa=_.Mf(_.rmb);this.Aa=_.Mf(_.smb);this.logger=null;const a=_.Mf(_.khb);this.fetch=a.fetch.bind(a)}R6a(a,b){if(this.Aa.getType(a.Dr())!==1)return new _.phb(a,null,0);var c=this.oa.policy;const d=_.Gq(a,ymb);var e=null;if(d){e={};if(d.zHb)for(var f of d.zHb)e[f]=!0;else if(c)for(var g of c.Ca())e[g]=!0;let p=1,q=0;f=Infinity;g=2;if(c){p=c.Ba()||p;let u;const A=(u=c.La())==null?void 0:u.getSeconds();q=_.ze(c.Ma().getSeconds())*1E3;f=A!=null?_.ze(A)*1E3:f;g=c.Aa()||g}var h,
k,l;let r;c=(h=d.maxAttempts)!=null?h:p;h=(k=d.Poc)!=null?k:q;k=(l=d.Jeb)!=null?l:g;l=(r=d.Ytc)!=null?r:f;e=new _.wmb(c,h,k,l,e,d.rzd)}else c&&(e=xmb(c));e&&e.pWa()?(b=zmb(this,a,b,e),a=new _.phb(a,b,2)):a=new _.phb(a,null,0);return a}},_.tmb);
_.x();
});
// Google Inc.
