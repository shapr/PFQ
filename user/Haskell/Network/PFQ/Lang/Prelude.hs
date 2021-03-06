--    Copyright (c) 2011-16, Nicola Bonelli
--    All rights reserved.
--
--    Redistribution and use in source and binary forms, with or without
--    modification, are permitted provided that the following conditions are met:
--
--    * Redistributions of source code must retain the above copyright notice,
--      this list of conditions and the following disclaimer.
--    * Redistributions in binary form must reproduce the above copyright
--      notice, this list of conditions and the following disclaimer in the
--      documentation and/or other materials provided with the distribution.
--    * Neither the name of University of Pisa nor the names of its contributors
--      may be used to endorse or promote products derived from this software
--      without specific prior written permission.
--
--    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
--    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
--    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
--    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
--    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
--    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
--    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
--    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
--    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
--    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
--    POSSIBILITY OF SUCH DAMAGE.
--
--

{-# LANGUAGE RebindableSyntax #-}

module Network.PFQ.Lang.Prelude
    ( ($)
    , (>>)
    , (>>=)
    , return
    , fromInteger
    , fromRational
    , fromString
    , ifThenElse
    ) where


import qualified Prelude as P hiding ((>>), (>>=), return)
import qualified Data.String as S
import Network.PFQ.Lang
import Network.PFQ.Lang.Default

a >> b = a >-> b


return  = P.undefined
_ >>= _ = P.undefined


fromInteger :: (P.Num a) => P.Integer -> a
fromInteger = P.fromInteger


fromRational :: (P.Fractional a) => P.Rational -> a
fromRational = P.fromRational

fromString :: (S.IsString a) => P.String -> a
fromString = S.fromString

ifThenElse = conditional

{-# INLINE ($) #-}
($)   :: (a -> b) -> a -> b
f $ x =  f x

