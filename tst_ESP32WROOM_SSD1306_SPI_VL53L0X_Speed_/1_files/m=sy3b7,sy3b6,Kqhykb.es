loaded_h_0(function(_){var window=this;
_.M6g=class extends _.m{constructor(a){super(a)}};_.N6g=class extends _.m{constructor(a){super(a)}};
_.I6g=function(a,b){return _.si(a,3,b)};_.J6g=function(a,b){return _.Oh(a,2,b)};_.L6g=class extends _.m{constructor(a){super(a)}Aa(a){return _.lj(this,5,_.K6g,a)}};_.K6g=[5,6];
_.v("Kqhykb");
var ukB=function(a,b){a=_.Yoa(a.getContext(0));try{const c=b();if(c)return _.bqa(c)}finally{_.Ff(a)}},vkB=class extends _.m{constructor(a){super(a)}},wkB=class extends _.m{constructor(a){super(a)}},xkB=function(){var a=new _.N6g;var b=new wkB;b=_.Qh(b,2,!0);return _.mj(a,1,[b])};var ykB=!!(_.Ci[72]&128);
_.Zf(class{constructor(){this.Ba=new _.wpa;this.oa=!1;this.logger=null}async canShowPromotion(a,b){return ykB&&this.oa?Promise.resolve(!1):this.Aa([{promoId:a,variantId:b}]).then(c=>{c=_.jj(c,wkB,1)[0];return!_.B(c,2)})}uBc(){this.oa=!0}async Aa(a){var b=(0,_.Of)(),c=b();b=b(1);try{try{const d=b(await c(ukB(this.Ba,()=>_.xpa(_.Apa(_.m0a))))),e=new _.M6g,f=a.map(g=>{var h=new vkB;h=_.si(h,1,g.promoId);g.variantId!==void 0&&_.Xg(h,2,g.variantId);return h});_.mj(e,3,f);return d.Aa(e)}catch(d){b()}return xkB()}finally{c()}}async EYa(a,b,
c){var d=(0,_.Of)(),e=d();d=d(1);try{try{const f=d(await e(ukB(this.Ba,()=>_.xpa(_.Apa(_.m0a))))),g=_.J6g(_.I6g(new _.L6g,a),b);c&&_.Xg(g,10,c);return f.EYa(g)}catch(f){d()}}finally{e()}}async recordPromoShown(a,b){this.oa=!0;return this.EYa(a,1,b)}async recordPromoDismissed(a,b){return this.EYa(a,3,b)}async recordPromoAccept(a,b){return this.EYa(a,2,b)}async renderPromo(a,b){_.Sg(document.body,"xgssSc",{promoId:a,variantId:b})}},_.p0a);
_.x();
});
// Google Inc.
