loaded_h_0(function(_){var window=this;
_.t_g=class extends _.m{constructor(a){super(a)}};_.u_g=class extends _.m{constructor(a){super(a)}};
_.p_g=function(a,b){return _.si(a,3,b)};_.q_g=function(a,b){return _.Ph(a,2,b)};_.s_g=class extends _.m{constructor(a){super(a)}Aa(a){return _.jj(this,5,_.r_g,a)}};_.r_g=[5,6];
_.v("Kqhykb");
var RQA=function(a,b){a=_.cpa(a.getContext(0));try{const c=b();if(c)return _.fqa(c)}finally{_.Hf(a)}},SQA=function(a,b){return _.mk(a,1,b)},TQA=class extends _.m{constructor(a){super(a)}},UQA=function(){var a=new _.u_g;var b=new TQA;b=_.Rh(b,2,!0);return _.kj(a,1,[b])};var VQA=!!(_.Bi[70]>>27&1);
_.bg(class{constructor(){this.Ba=new _.Bpa;this.oa=!1;this.logger=null}async canShowPromotion(a){return VQA&&this.oa?Promise.resolve(!1):this.Aa([a]).then(b=>{b=_.gj(b,TQA,1)[0];return!_.B(b,2)})}gBc(){this.oa=!0}async Aa(a){var b=(0,_.Rf)(),c=b();b=b(1);try{try{return b(await c(RQA(this.Ba,()=>_.Cpa(_.Fpa(_.Q0a))))).Aa(SQA(new _.t_g,a))}catch(d){b()}return UQA()}finally{c()}}async vYa(a,b){var c=(0,_.Rf)(),d=c();c=c(1);try{try{return c(await d(RQA(this.Ba,()=>_.Cpa(_.Fpa(_.Q0a))))).vYa(_.q_g(_.p_g(new _.s_g,a),
b))}catch(e){c()}}finally{d()}}async recordPromoShown(a){this.oa=!0;return this.vYa(a,1)}async recordPromoDismissed(a){return this.vYa(a,3)}async recordPromoAccept(a){return this.vYa(a,2)}async renderPromo(a){_.Ug(document.body,"xgssSc",{promoId:a})}},_.T0a);
_.x();
});
// Google Inc.
