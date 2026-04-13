loaded_h_0(function(_){var window=this;
_.v("lOO0Vd");
_.ymb=new _.MOa(_.iSa);
_.x();
var Amb;Amb=function(a){if(a.kMc){const b=Date.now()-a.Afe;return a.kMc(a.CGb+1,b)}return Math.random()*Math.min(a.HSd*Math.pow(a.Eac,a.CGb),a.C_d)};_.Bmb=function(a){if(!a.JVa())throw Error("Ue`"+a.Klb);++a.CGb;a.Dac=Amb(a)};_.Cmb=class{constructor(a,b,c,d,e,f){this.Klb=a;this.HSd=b;this.Eac=c;this.C_d=d;this.m9d=e;this.kMc=f||null;this.Afe=Date.now();this.CGb=0;this.Dac=Amb(this)}CWc(){return this.CGb}JVa(a){return this.CGb>=this.Klb?!1:a!=null?!!this.m9d[a]:!0}};
_.v("P6sQOc");
var Dmb=function(a){const b={};_.Ka(a.Ca(),e=>{b[e]=!0});const c=a.Da(),d=a.Ha();return new _.Cmb(a.Ba(),_.we(c.getSeconds())*1E3,a.Aa(),_.we(d.getSeconds())*1E3,b)},Emb=new _.Bq("retryConfigOverrides"),Fmb=function(a,b,c,d){return c.then(e=>e,e=>{if(e instanceof _.ii){if(!e.status||!d.JVa(e.status.Sr()))throw e;}else if("function"==typeof _.Hib&&e instanceof _.Hib&&e.oa!==103&&e.oa!==7)throw e;if(d&&!d.JVa())return _.yh(e);const f=d.Dac;return(new _.Lg(g=>{setTimeout(g,f)})).then(()=>{_.Bmb(d);const g=
d.CWc();b=b.dC(_.YWa,g);return Fmb(a,b,a.fetch(b),d)})})};
_.Zf(class{constructor(){this.oa=_.Mf(_.xmb);this.Ba=_.Mf(_.ymb);this.logger=null;const a=_.Mf(_.shb);this.fetch=a.fetch.bind(a)}Aa(a,b){if(this.Ba.getType(a.zr())!==1)return _.zhb(a);var c=this.oa.policy;const d=_.Cq(a,Emb);var e=null;if(d){e={};if(d.EGb)for(var f of d.EGb)e[f]=!0;else if(c)for(var g of c.Ca())e[g]=!0;let p=1,q=0;f=Infinity;g=2;if(c){p=c.Ba()||p;let u;const A=(u=c.La())==null?void 0:u.getSeconds();q=_.we(c.Ma().getSeconds())*1E3;f=A!=null?_.we(A)*1E3:f;g=c.Aa()||g}var h,k,l;let r;
c=(h=d.maxAttempts)!=null?h:p;h=(k=d.Wnc)!=null?k:q;k=(l=d.Vdb)!=null?l:g;l=(r=d.ctc)!=null?r:f;e=new _.Cmb(c,h,k,l,e,d.fyd)}else c&&(e=Dmb(c));e&&e.JVa()?(b=Fmb(this,a,b,e),a=new _.vhb(a,b,2)):a=_.zhb(a);return a}},_.zmb);
_.x();
});
// Google Inc.
